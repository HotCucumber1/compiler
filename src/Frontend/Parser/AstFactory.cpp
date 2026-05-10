#include "AstFactory.h"

Token GetToken(const std::shared_ptr<AstNode>& node);
std::shared_ptr<AstNode> ProcessUndefinedNode(
	SymbolId id,
	const std::vector<std::shared_ptr<AstNode>>& children);

// TODO все индексы в константы

std::shared_ptr<AstNode> AstFactory::CreateNode(
	AstNodeType type,
	SymbolId id,
	const std::vector<std::shared_ptr<AstNode>>& children)
{
	if (type == AstNodeType::UNDEFINED)
	{
		return ProcessUndefinedNode(id, children);
	}
	switch (type)
	{
	case AstNodeType::PROGRAM:
		return std::make_shared<CommonNode>(
			id, AstNodeType::PROGRAM,
			std::dynamic_pointer_cast<CommonNode>(children[0])
				? std::dynamic_pointer_cast<CommonNode>(children[0])->children
				: children);
		// case AstNodeType::BLOCK: {
		// 	auto container = std::make_shared<CommonNode>(id, type, std::vector<std::shared_ptr<AstNode>>{});
		// 	for (auto& child : children)
		// 	{
		// 		if (!child)
		// 		{
		// 			continue;
		// 		}
		// 		if (const auto commonChild = std::dynamic_pointer_cast<CommonNode>(child))
		// 		{
		// 			if (commonChild->type == type)
		// 			{
		// 				container->children.insert(
		// 					container->children.end(),
		// 					commonChild->children.begin(),
		// 					commonChild->children.end());
		// 				continue;
		// 			}
		// 		}
		// 		if (const auto term = std::dynamic_pointer_cast<TerminalNode>(child))
		// 		{
		// 			if (term->token.value == ";")
		// 			{
		// 				continue;
		// 			}
		// 		}
		// 		container->children.push_back(child);
		// 	}
		// 	return container;
		// }

	case AstNodeType::VAR_DECL:
		return std::make_shared<VarDeclNode>(id, GetToken(children[1]).value, children[3], false);

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

	case AstNodeType::CALL:
		// ID ( Expression )
		return std::make_shared<CallNode>(id, GetToken(children[0]).value, children[2]);

	default:
		throw std::runtime_error("AstFactory Error: AstNodeType is not being processed " + std::to_string(static_cast<int>(type)));
		// default:
		// 	return std::make_shared<CommonNode>(id, type, children);
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

std::shared_ptr<AstNode> ProcessUndefinedNode(
	SymbolId id,
	const std::vector<std::shared_ptr<AstNode>>& children)
{
	if (children.empty())
	{
		return nullptr;
	}
	if (children.size() >= 2 && children[1]->type == AstNodeType::TERMINAL && GetToken(children[1]).value == ";")
	{
		auto block = std::make_shared<CommonNode>(id, AstNodeType::BLOCK, std::vector<std::shared_ptr<AstNode>>{});
		block->children.push_back(children[0]);

		if (children.size() == 3 && children[2])
		{
			if (const auto tail = std::dynamic_pointer_cast<CommonNode>(children[2]))
			{
				block->children.insert(block->children.end(), tail->children.begin(), tail->children.end());
			}
			else
			{
				block->children.push_back(children[2]);
			}
		}
		return block;
	}

	if (children.size() == 3
		&& GetToken(children[0]).value == "(")
	{
		return children[1];
	}

	return children[0];
}
