#include "Parser.h"
#include "AstFactory.h"

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
			throw SyntaxError({
								  currentToken.file,
								  currentToken.line,
								  currentToken.pos,
							  },
				"Unexpected token '" + currentToken.value + "'");
		}

		const auto action = it->second;

		if (action.type == ActionType::Shift)
		{
			stateStack.push_back(action.value);

			auto leaf = std::make_shared<TerminalNode>(currentSymbolId, currentToken);
			nodeStack.push_back(leaf);

			if (!m_lexer.IsEmpty())
			{
				currentToken = m_lexer.GetToken();
				currentSymbolId = GetSymbolId(currentToken);
			}
			else
			{
				currentToken = Token(
					currentToken.pos,
					currentToken.line,
					TokenType::END_OF_FILE,
					"EOF",
					currentToken.file);
				currentSymbolId = m_builder.GetSymbolId({
					Grammar::END_OF_FILE,
					true,
				});
			}
		}
		else if (action.type == ActionType::Reduce)
		{
			Reduce(stateStack, nodeStack, action);
		}
		else if (action.type == ActionType::Accept)
		{
			return nodeStack.back();
		}
	}
}

void Parser::Reduce(
	std::vector<int>& stateStack,
	std::vector<std::shared_ptr<AstNode>>& nodeStack,
	const Action& action) const
{
	const auto& rule = m_rules[action.value];

	std::vector<std::shared_ptr<AstNode>> children(rule.rhsSize);
	for (int i = static_cast<int>(rule.rhsSize) - 1; i >= 0; --i)
	{
		children[i] = nodeStack.back();
		nodeStack.pop_back();
		stateStack.pop_back();
	}

	const auto parentNode = AstFactory::CreateNode(rule.nodeType, rule.lhsId, children);

	const auto stateAfterPop = stateStack.back();
	const auto gotoIt = m_table[stateAfterPop].find(rule.lhsId);

	if (gotoIt == m_table[stateAfterPop].end())
	{
		throw std::runtime_error("Parser error: SLR table GOTO transition failed after reduce on state "
			+ std::to_string(stateAfterPop));
	}

	stateStack.push_back(gotoIt->second.value);
	nodeStack.push_back(parentNode);
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
			throw SyntaxError({
								  token.file,
								  token.line,
								  token.pos,
							  },
				"Undefined toke: " + token.value);
		}
	}

	const RuleItem terminalItem{
		terminalName,
		true,
	};
	const auto id = m_builder.GetSymbolId(terminalItem);

	if (id == -1)
	{
		throw SemanticError({
								token.file,
								token.line,
								token.pos,
							},
			"Lexical error: token type '" + terminalName + "' is not defined in the Grammar!");
	}
	return id;
}
