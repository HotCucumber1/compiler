#include "AstFactory.h"

Token GetToken(const std::shared_ptr<AstNode>& node);
std::shared_ptr<AstNode> AppendToList(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);

// TODO все индексы в константы

std::shared_ptr<AstNode> AstFactory::CreateNode(
	AstNodeType type,
	SymbolId id,
	const std::vector<std::shared_ptr<AstNode>>& children)
{
	if (type == AstNodeType::UNDEFINED)
	{
		return children.empty() ? nullptr : children[0];
	}
	switch (type)
	{
	case AstNodeType::PROGRAM:
		if (!children.empty())
		{
			if (auto block = std::dynamic_pointer_cast<CommonNode>(children[0]))
			{
				return std::make_shared<CommonNode>(id, AstNodeType::PROGRAM, block->children);
			}
		}
		return std::make_shared<CommonNode>(id, AstNodeType::PROGRAM, children);

	case AstNodeType::VAR_DECL: {
		auto val = (children.size() < 3) ? nullptr : children[3];
		return std::make_shared<VarDeclNode>(id, GetToken(children[1]).value, val, false);
	}

	case AstNodeType::CONST_DECL:
		if (children.size() == 2)
		{
			return children[1];
		}
		return std::make_shared<VarDeclNode>(id, GetToken(children[1]).value, children[3], true);

	case AstNodeType::ASSIGNMENT:
		return std::make_shared<AssignmentNode>(id, GetToken(children[0]).value, children[2]);

	case AstNodeType::BINARY_OP:
		return std::make_shared<BinaryOpNode>(id, children[0], GetToken(children[1]), children[2]);

	case AstNodeType::UNARY_OP:
		return std::make_shared<UnaryOpNode>(id, GetToken(children[0]), children[1]);

	case AstNodeType::LITERAL:
		return std::make_shared<LiteralNode>(id, GetToken(children[0]));

	case AstNodeType::VAR_ACCESS:
		return std::make_shared<VarAccessNode>(id, GetToken(children[0]).value);

	case AstNodeType::IF_STATEMENT: {
		// if ( Exp ) { List } -> size 7
		// if ( Exp ) { List } else { List } -> size 11
		// if ( Exp ) { List } else IfStmt -> size 9
		auto cond = children[2];
		auto thenB = children[5];
		std::shared_ptr<AstNode> elseB = nullptr;
		if (children.size() == 11)
		{
			elseB = children[9];
		}
		else if (children.size() == 9)
		{
			elseB = children[8];
		}
		return std::make_shared<IfNode>(id, cond, thenB, elseB);
	}

	case AstNodeType::WHILE_STATEMENT:
		// while ( Exp ) { List }
		return std::make_shared<WhileNode>(id, children[2], children[5]);

	case AstNodeType::FOR_STATEMENT:
		// for ( Decl ; Cond ; Incr ) { List } -> size 11
		return std::make_shared<ForNode>(id, children[2], children[4], children[6], children[9]);

	case AstNodeType::CALL: {
		std::string name = GetToken(children[0]).value;
		std::vector<std::shared_ptr<AstNode>> args;

		if (const auto argList = std::dynamic_pointer_cast<CommonNode>(children[2]))
		{
			args = argList->children;
		}
		return std::make_shared<CallNode>(id, name, args);
	}

	case AstNodeType::FUNC_DECL: {
		std::string name = GetToken(children[1]).value;
		std::vector<std::string> params;

		if (auto paramList = std::dynamic_pointer_cast<CommonNode>(children[3]))
		{
			for (auto& child : paramList->children)
			{
				if (const auto pElem = std::dynamic_pointer_cast<VarDeclNode>(child))
				{
					params.push_back(pElem->varName);
				}
			}
		}
		auto body = children[6];
		return std::make_shared<FunctionDeclNode>(id, name, params, body);
	}

	case AstNodeType::LIST_APPEND:
	case AstNodeType::LIST_SINGLE:
		return AppendToList(id, children);

	case AstNodeType::PASS_THROUGH:
		if (children.empty())
			return nullptr;
		if (children.size() == 3 && GetToken(children[0]).value == "(")
		{
			return children[1];
		}
		return children[0];

	default:
		throw std::runtime_error("AstFactory Error: AstNodeType is not being processed " + std::to_string(static_cast<int>(type)));
	}
}

Token GetToken(const std::shared_ptr<AstNode>& node)
{
	auto term = std::dynamic_pointer_cast<TerminalNode>(node);
	if (term)
	{
		return term->token;
	}
	throw std::runtime_error("AST Builder error: Expected TerminalNode!");
}

std::shared_ptr<AstNode> AppendToList(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	auto block = std::make_shared<CommonNode>(id, AstNodeType::BLOCK, std::vector<std::shared_ptr<AstNode>>{});

	if (children[0])
	{
		block->children.push_back(children[0]);
	}

	if (children.size() > 1 && children[1])
	{
		if (auto tail = std::dynamic_pointer_cast<CommonNode>(children[1]))
		{
			block->children.insert(block->children.end(), tail->children.begin(), tail->children.end());
		}
		else
		{
			block->children.push_back(children[1]);
		}
	}
	return block;
}
