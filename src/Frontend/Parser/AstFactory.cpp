#include "AstFactory.h"

Token GetToken(const std::shared_ptr<AstNode>& node);

// TODO все индексы в константы

std::shared_ptr<AstNode> AstFactory::CreateNode(
	AstNodeType type,
	SymbolId id,
	const std::vector<std::shared_ptr<AstNode>>& children)
{
	switch (type)
	{
	case AstNodeType::BINARY_OP:
		return std::make_shared<BinaryOpNode>(id, children[0], GetToken(children[1]), children[2]);

	case AstNodeType::UNARY_OP:
		return std::make_shared<UnaryOpNode>(id, GetToken(children[0]), children[1]);

	case AstNodeType::VAR_DECL:
	case AstNodeType::CONST_DECL: {
		const auto isConst = (type == AstNodeType::CONST_DECL);
		const auto name = GetToken(children[1]).value;
		const auto expr = (children.size() > 3)
			? children[3]
			: nullptr;
		return std::make_shared<VarDeclNode>(id, name, expr, isConst);
	}
	case AstNodeType::ASSIGNMENT:
		return std::make_shared<AssignmentNode>(id, GetToken(children[0]).value, children[2]);

	case AstNodeType::LITERAL:
		return std::make_shared<LiteralNode>(id, GetToken(children[0]));

	case AstNodeType::VAR_ACCESS:
		return std::make_shared<VarAccessNode>(id, GetToken(children[0]).value);

	case AstNodeType::IF_STATEMENT: {
		auto cond = children[2];
		auto thenBranch = children[5];
		std::shared_ptr<AstNode> elseBranch = nullptr;
		if (children.size() == 9)
		{
			elseBranch = children[8];
		}
		else if (children.size() >= 11)
		{
			elseBranch = children[9];
		}
		return std::make_shared<IfNode>(id, cond, thenBranch, elseBranch);
	}

	case AstNodeType::WHILE_STATEMENT:
		return std::make_shared<WhileNode>(id, children[2], children[5]);

	case AstNodeType::FOR_STATEMENT:
		return std::make_shared<ForNode>(
			id,
			children[2],
			(children.size() > 4 ? children[4] : nullptr),
			(children.size() > 6 ? children[6] : nullptr),
			(children.size() > 9 ? children[9] : nullptr));

	case AstNodeType::CALL:
		return std::make_shared<CallNode>(
			id,
			GetToken(children[0]).value,
			(children.size() > 2
					? children[2]
					: nullptr));

	default:
		return std::make_shared<CommonNode>(id, type, children);
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
