#include "Parser.h"

Parser::Parser(const SLRTableBuilder& tableBuilder, Lexer& lexer)
	: m_table(tableBuilder.GetTable())
	, m_rules(tableBuilder.GetRules())
	, m_builder(tableBuilder)
	, m_lexer(lexer)
{
}

std::shared_ptr<AstNode> Parser::Parse() const
{
	std::vector stateStack = { 0 };
	std::vector<std::shared_ptr<AstNode>> nodeStack;

	if (m_lexer.IsEmpty())
	{
		return nullptr;
	}

	auto currentToken = m_lexer.GetToken();
	auto currentSymbolId = GetSymbolId(currentToken);

	while (true)
	{
		const auto currentState = stateStack.back();

		auto it = m_table[currentState].find(currentSymbolId);
		if (it == m_table[currentState].end())
		{
			throw std::runtime_error(
				"Syntax error (" + std::to_string(currentToken.line) + ':' + std::to_string(currentToken.pos)
				+ "): Unexpected token '" + currentToken.value + "'");
		}

		const auto action = it->second;

		if (action.type == ActionType::Shift) // TODO Parse Tree -> AST
		{
			stateStack.push_back(action.value);

			auto leaf = std::make_shared<AstNode>(currentSymbolId, currentToken, AstNodeType::TERMINAL);
			nodeStack.push_back(leaf);

			if (!m_lexer.IsEmpty())
			{
				currentToken = m_lexer.GetToken();
				currentSymbolId = GetSymbolId(currentToken);
			}
			else
			{
				currentSymbolId = m_builder.GetSymbolId({
					Grammar::END_OF_FILE,
					true,
				});
			}
		}
		else if (action.type == ActionType::Reduce)
		{
			const auto& rule = m_rules[action.value];
			auto parentNode = std::make_shared<AstNode>(rule.lhsId, rule.nodeType);

			for (size_t i = 0; i < rule.rhsSize; ++i)
			{
				parentNode->children.insert(parentNode->children.begin(), nodeStack.back());
				nodeStack.pop_back();
				stateStack.pop_back();
			}

			const auto stateAfterPop = stateStack.back();
			auto gotoIt = m_table[stateAfterPop].find(rule.lhsId);

			if (gotoIt == m_table[stateAfterPop].end())
			{
				throw std::runtime_error("Parser PANIC: GOTO transition failed after reduce!");
			}

			stateStack.push_back(gotoIt->second.value);
			nodeStack.push_back(parentNode);
		}
		else if (action.type == ActionType::Accept)
		{
			return nodeStack.back();
		}
	}
}

SymbolId Parser::GetSymbolId(const Token& token) const
{
	std::string terminalName = TOKEN_NAMES.at(TokenType::IDENTIFIER);
	if (REVERSE_KEYWORDS.contains(token.type))
	{
		terminalName = REVERSE_KEYWORDS.at(token.type);
	}
	else if (REVERSE_SPECIAL_CHARS.contains(token.type))
	{
		terminalName = REVERSE_SPECIAL_CHARS.at(token.type);
	}
	else if (REVERSE_SPECIAL_CHARS_DOUBLE.contains(token.type))
	{
		terminalName = REVERSE_SPECIAL_CHARS_DOUBLE.at(token.type);
	}
	else
	{
		switch (token.type)
		{
		case TokenType::INT:
		case TokenType::FLOAT:
		case TokenType::IDENTIFIER:
		case TokenType::STRING:
		case TokenType::TRUE:
		case TokenType::FALSE:
			terminalName = TOKEN_NAMES.at(token.type);
			break;
		default:
			throw std::runtime_error("Unknown token type in Parser: " + std::to_string(static_cast<int>(token.type)));
		}
	}

	const RuleItem terminalItem{
		terminalName,
		true,
	};
	const auto id = m_builder.GetSymbolId(terminalItem);

	if (id == -1)
	{
		throw std::runtime_error("Lexical error: token type '" + terminalName + "' is not defined in the Grammar!");
	}
	return id;
}
