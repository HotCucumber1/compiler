#include "AstFactory.h"

Token GetToken(const std::shared_ptr<AstNode>& node);
std::shared_ptr<AstNode> AppendToList(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::vector<std::shared_ptr<AstNode>> UnwrapList(const std::shared_ptr<AstNode>& node);
std::pair<DataType, bool> ExtractTypeInfo(const std::shared_ptr<AstNode>& typeNode);

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
	case AstNodeType::TYPE_INT:
	case AstNodeType::TYPE_FLOAT:
	case AstNodeType::TYPE_BOOL:
	case AstNodeType::TYPE_STRING:
	case AstNodeType::TYPE_VOID:
	case AstNodeType::TYPE_INT_ARRAY:
	case AstNodeType::TYPE_FLOAT_ARRAY:
	case AstNodeType::TYPE_BOOL_ARRAY:
	case AstNodeType::TYPE_STRING_ARRAY:
		return std::make_shared<CommonNode>(id, type, children);

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
		auto [dataType, isArray] = ExtractTypeInfo(children[0]);
		auto val = (children.size() < 4)
			? nullptr
			: children[3];

		return std::make_shared<VarDeclNode>(
			id,
			GetToken(children[1]).value,
			val,
			false,
			isArray,
			dataType);
	}

	case AstNodeType::CONST_DECL:
		if (children.size() >= 2)
		{
			if (auto varDecl = std::dynamic_pointer_cast<VarDeclNode>(children[1]))
			{
				varDecl->isConst = true;
				return varDecl;
			}
		}
		throw std::runtime_error("AST Factory error: Invalid CONST_DECL");

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
		auto [dataType, isArray] = ExtractTypeInfo(children[6]);
		return std::make_shared<FunctionDeclNode>(id, name, params, children[8], dataType, isArray);
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

	case AstNodeType::STRUCT_DECL: {
		auto name = GetToken(children[1]).value;
		return std::make_shared<StructDeclNode>(id, name, UnwrapList(children[3])); // TODO передавать не это
	}

	case AstNodeType::STRUCT_INIT: {
		auto name = GetToken(children[0]).value;
		return std::make_shared<StructInitNode>(id, name, UnwrapList(children[2]));
	}

	case AstNodeType::MEMBER_ACCESS:
		return std::make_shared<MemberAccessNode>(id, children[0], GetToken(children[2]).value);

	case AstNodeType::MEMBER_ASSIGNMENT:
		return std::make_shared<MemberAssignmentNode>(id, children[0], GetToken(children[2]).value, children[4]);

	case AstNodeType::ARRAY_LITERAL: {
		if (children.size() < 3)
		{
			throw std::runtime_error("AST Factory: ARRAY_LITERAL expects at least 3 children");
		}

		std::vector<std::shared_ptr<AstNode>> elements;
		if (const auto argList = std::dynamic_pointer_cast<CommonNode>(children[1]))
		{
			elements = argList->children;
		}
		else if (children[1])
		{
			elements.push_back(children[1]);
		}

		return std::make_shared<ArrayLiteralNode>(id, std::move(elements));
	}

	case AstNodeType::INDEX_ACCESS: {
		if (children.size() < 4)
		{
			throw std::runtime_error("AST Factory: INDEX_ACCESS expects at least 4 children");
		}
		return std::make_shared<IndexAccessNode>(id, children[0], children[2]);
	}

	case AstNodeType::INDEX_ASSIGN: {
		if (children.size() < 6)
		{
			throw std::runtime_error("AST Factory: INDEX_ASSIGN expects at least 6 children");
		}
		return std::make_shared<IndexAssignmentNode>(id, children[0], children[2], children[5]);
	}

	case AstNodeType::RETURN_STATEMENT: {
		std::shared_ptr<AstNode> expr = nullptr;
		if (children.size() == 2)
		{
			expr = children[1];
		}
		return std::make_shared<ReturnNode>(id, expr);
	}

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

	if (children.size() == 1)
	{
		if (children[0])
		{
			block->children.push_back(children[0]);
		}
	}
	else if (children.size() >= 2)
	{
		if (children[0])
		{
			block->children.push_back(children[0]);
		}
		auto tail = UnwrapList(children[children.size() - 1]);
		block->children.insert(block->children.end(), tail.begin(), tail.end());
	}
	return block;
}

std::vector<std::shared_ptr<AstNode>> UnwrapList(const std::shared_ptr<AstNode>& node)
{
	if (!node)
	{
		return {};
	}
	if (const auto common = std::dynamic_pointer_cast<CommonNode>(node))
	{
		if (common->type == AstNodeType::BLOCK)
		{
			return common->children;
		}
	}
	return { node };
}

/**
 *
 * @return <DataType, isArray>
 */
std::pair<DataType, bool> ExtractTypeInfo(const std::shared_ptr<AstNode>& typeNode)
{
	if (!typeNode)
	{
		return { DataType::UNKNOWN, false };
	}

	switch (typeNode->type)
	{
	case AstNodeType::TYPE_INT:
		return { DataType::INT, false };
	case AstNodeType::TYPE_FLOAT:
		return { DataType::FLOAT, false };
	case AstNodeType::TYPE_BOOL:
		return { DataType::BOOL, false };
	case AstNodeType::TYPE_STRING:
		return { DataType::STRING, false };
	case AstNodeType::TYPE_VOID:
		return { DataType::VOID, false };

	case AstNodeType::TYPE_INT_ARRAY:
		return { DataType::INT, true };
	case AstNodeType::TYPE_FLOAT_ARRAY:
		return { DataType::FLOAT, true };
	case AstNodeType::TYPE_BOOL_ARRAY:
		return { DataType::BOOL, true };
	case AstNodeType::TYPE_STRING_ARRAY:
		return { DataType::STRING, true };

	default:
		return { DataType::UNKNOWN, false };
	}
}
