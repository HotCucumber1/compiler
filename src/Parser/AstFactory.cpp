#include "AstFactory.h"

struct TypeInfo
{
	DataType type = DataType::UNKNOWN;
	int depth = 0;
	std::string structName;
};

Token GetToken(const std::shared_ptr<AstNode>& node);
std::shared_ptr<AstNode> AppendToList(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::vector<std::shared_ptr<AstNode>> UnwrapList(const std::shared_ptr<AstNode>& node);
TypeInfo ExtractTypeInfo(const std::shared_ptr<AstNode>& typeNode);
SourceLocation ExtractLocation(const std::vector<std::shared_ptr<AstNode>>& children);

// TODO все индексы в константы
// TODO разбить на функции

std::shared_ptr<AstNode> AstFactory::CreateNode(
	AstNodeType type,
	SymbolId id,
	const std::vector<std::shared_ptr<AstNode>>& children)
{
	if (type == AstNodeType::UNDEFINED)
	{
		return children.empty() ? nullptr : children[0];
	}

	std::shared_ptr<AstNode> result = nullptr;
	try
	{
		switch (type)
		{
		case AstNodeType::TYPE_INT:
		case AstNodeType::TYPE_FLOAT:
		case AstNodeType::TYPE_BOOL:
		case AstNodeType::TYPE_STRING:
		case AstNodeType::TYPE_VOID:
		case AstNodeType::TYPE_CUSTOM:
		case AstNodeType::TYPE_ARRAY:
			result = std::make_shared<CommonNode>(id, type, children);
			break;

		case AstNodeType::PROGRAM:
			if (!children.empty())
			{
				if (auto block = std::dynamic_pointer_cast<CommonNode>(children[0]))
				{
					result = std::make_shared<CommonNode>(id, AstNodeType::PROGRAM, block->children);
					break;
				}
			}
			result = std::make_shared<CommonNode>(id, AstNodeType::PROGRAM, children);
			break;

		case AstNodeType::VAR_DECL: {
			auto [dataType, depth, structName] = ExtractTypeInfo(children[0]);
			auto val = (children.size() < 4)
				? nullptr
				: children[3];

			result = std::make_shared<VarDeclNode>(
				id,
				GetToken(children[1]).value,
				val,
				false,
				depth,
				dataType,
				structName);
			break;
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
			result = std::make_shared<AssignmentNode>(id, GetToken(children[0]).value, children[2]);
			break;

		case AstNodeType::BINARY_OP:
			result = std::make_shared<BinaryOpNode>(id, children[0], GetToken(children[1]), children[2]);
			break;

		case AstNodeType::UNARY_OP:
			result = std::make_shared<UnaryOpNode>(id, GetToken(children[0]), children[1]);
			break;

		case AstNodeType::LITERAL:
			result = std::make_shared<LiteralNode>(id, GetToken(children[0]));
			break;

		case AstNodeType::VAR_ACCESS:
			result = std::make_shared<VarAccessNode>(id, GetToken(children[0]).value);
			break;

		case AstNodeType::IF_STATEMENT: {
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
			result = std::make_shared<IfNode>(id, cond, thenB, elseB);
			break;
		}

		case AstNodeType::WHILE_STATEMENT:
			result = std::make_shared<WhileNode>(id, children[2], children[5]);
			break;

		case AstNodeType::FOR_STATEMENT:
			result = std::make_shared<ForNode>(id, children[2], children[4], children[6], children[9]);
			break;

		case AstNodeType::CALL: {
			std::string name = GetToken(children[0]).value;
			std::vector<std::shared_ptr<AstNode>> args;

			if (const auto argList = std::dynamic_pointer_cast<CommonNode>(children[2]))
			{
				args = argList->children;
			}
			result = std::make_shared<CallNode>(id, name, args);
			break;
		}

		case AstNodeType::FUNC_DECL: {
			const auto name = GetToken(children[1]).value;

			std::vector<std::shared_ptr<VarDeclNode>> params;
			if (auto paramList = std::dynamic_pointer_cast<CommonNode>(children[3]))
			{
				if (paramList->type == AstNodeType::BLOCK)
				{
					for (auto& child : paramList->children)
					{
						if (const auto pElem = std::dynamic_pointer_cast<VarDeclNode>(child))
						{
							params.push_back(pElem);
						}
					}
				}
			}

			std::vector<std::shared_ptr<VarDeclNode>> captures;
			if (auto useList = std::dynamic_pointer_cast<CommonNode>(children[5]))
			{
				if (useList->type == AstNodeType::BLOCK)
				{
					for (auto& child : useList->children)
					{
						if (const auto cElem = std::dynamic_pointer_cast<VarDeclNode>(child))
							captures.push_back(cElem);
					}
				}
			}

			auto [dataType, depth, structName] = ExtractTypeInfo(children[7]);
			auto body = children[9];

			result = std::make_shared<FunctionDeclNode>(
				id,
				name,
				params,
				captures,
				body,
				dataType,
				depth,
				structName);
			break;
		}

		case AstNodeType::USE_DECL:
			return children[2];

		case AstNodeType::LIST_APPEND:
		case AstNodeType::LIST_SINGLE:
			return AppendToList(id, children);

		case AstNodeType::PASS_THROUGH:
			if (children.empty())
			{
				return nullptr;
			}
			if (children.size() == 3)
			{
				if (auto term = std::dynamic_pointer_cast<TerminalNode>(children[0]))
				{
					if (term->token.value == "(")
					{
						return children[1];
					}
				}
			}
			return children[0];

		case AstNodeType::STRUCT_DECL: {
			auto name = GetToken(children[1]).value;
			result = std::make_shared<StructDeclNode>(id, name, UnwrapList(children[3]));
			break;
		}

		case AstNodeType::STRUCT_INIT: {
			auto name = GetToken(children[0]).value;
			result = std::make_shared<StructInitNode>(id, name, UnwrapList(children[2]));
			break;
		}

		case AstNodeType::MEMBER_ACCESS:
			result = std::make_shared<MemberAccessNode>(id, children[0], GetToken(children[2]).value);
			break;

		case AstNodeType::MEMBER_ASSIGNMENT:
			result = std::make_shared<MemberAssignmentNode>(id, children[0], GetToken(children[2]).value, children[4]);
			break;

		case AstNodeType::ARRAY_LITERAL: {
			if (children.size() < 3)
			{
				throw std::runtime_error("AST Factory: ARRAY_LITERAL expects at least 3 children");
			}

			std::vector<std::shared_ptr<AstNode>> elements;
			if (const auto& argNode = children[1])
			{
				if (auto common = std::dynamic_pointer_cast<CommonNode>(argNode))
				{
					if (common->type == AstNodeType::BLOCK
						|| common->type == AstNodeType::LIST_SINGLE
						|| common->type == AstNodeType::LIST_APPEND)
					{
						elements = common->children;
					}
					else
					{
						elements.push_back(argNode);
					}
				}
				else
				{
					elements.push_back(argNode);
				}
			}
			else if (children[1])
			{
				elements.push_back(children[1]);
			}

			result = std::make_shared<ArrayLiteralNode>(id, std::move(elements));
			break;
		}

		case AstNodeType::INDEX_ACCESS: {
			if (children.size() < 4)
			{
				throw std::runtime_error("AST Factory: INDEX_ACCESS expects at least 4 children");
			}
			result = std::make_shared<IndexAccessNode>(id, children[0], children[2]);
			break;
		}

		case AstNodeType::INDEX_ASSIGN: {
			if (children.size() < 6)
			{
				throw std::runtime_error("AST Factory: INDEX_ASSIGN expects at least 6 children");
			}
			result = std::make_shared<IndexAssignmentNode>(id, children[0], children[2], children[5]);
			break;
		}

		case AstNodeType::RETURN_STATEMENT: {
			std::shared_ptr<AstNode> expr = nullptr;
			if (children.size() == 2)
			{
				expr = children[1];
			}
			result = std::make_shared<ReturnNode>(id, expr);
			break;
		}

		case AstNodeType::IMPORT_DECL: {
			auto rawPath = GetToken(children[1]).value;
			auto cleanPath = rawPath.substr(1, rawPath.size() - 2);
			result = std::make_shared<ImportNode>(id, cleanPath);
			break;
		}

		case AstNodeType::SCHEMA_FUNC: {
			const auto name = GetToken(children[1]).value;

			std::vector<std::shared_ptr<VarDeclNode>> params;
			if (auto paramList = std::dynamic_pointer_cast<CommonNode>(children[3]))
			{
				if (paramList->type == AstNodeType::BLOCK)
				{
					for (auto& child : paramList->children)
					{
						if (const auto pElem = std::dynamic_pointer_cast<VarDeclNode>(child))
						{
							params.push_back(pElem);
						}
					}
				}
			}
			const auto returnState = GetToken(children[6]).value;

			result = std::make_shared<SchemaFuncNode>(id, name, params, returnState);
			break;
		}

		case AstNodeType::SCHEMA_STATE: {
			const auto name = GetToken(children[1]).value;

			std::vector<std::shared_ptr<VarDeclNode>> fields;
			if (auto paramList = std::dynamic_pointer_cast<CommonNode>(children[3]))
			{
				if (paramList->type == AstNodeType::BLOCK)
				{
					for (auto& child : paramList->children)
					{
						if (const auto pElem = std::dynamic_pointer_cast<VarDeclNode>(child))
						{
							fields.push_back(pElem);
						}
					}
				}
			}

			std::vector<std::shared_ptr<SchemaFuncNode>> methods;
			if (auto methodList = std::dynamic_pointer_cast<CommonNode>(children[6]))
			{
				if (methodList->type == AstNodeType::BLOCK)
				{
					for (auto& child : methodList->children)
					{
						if (const auto mElem = std::dynamic_pointer_cast<SchemaFuncNode>(child))
						{
							methods.push_back(mElem);
						}
					}
				}
			}

			result = std::make_shared<SchemaStateNode>(id, name, fields, methods);
			break;
		}

		case AstNodeType::SCHEMA_DECL: {
			const auto name = GetToken(children[1]).value;

			std::vector<std::shared_ptr<SchemaStateNode>> states;
			if (auto stateList = std::dynamic_pointer_cast<CommonNode>(children[3]))
			{
				if (stateList->type == AstNodeType::BLOCK)
				{
					for (auto& child : stateList->children)
					{
						if (const auto sElem = std::dynamic_pointer_cast<SchemaStateNode>(child))
						{
							states.push_back(sElem);
						}
					}
				}
			}

			result = std::make_shared<SchemaDeclNode>(id, name, states);
			break;
		}

		case AstNodeType::IMPL_STATE_TYPE: {
			const auto schema = GetToken(children[0]).value;
			const auto state = GetToken(children[2]).value;
			result = std::make_shared<ImplStateNode>(id, schema, state);
			break;
		}

		case AstNodeType::TSOP_INIT: {
			auto stateNode = std::dynamic_pointer_cast<ImplStateNode>(children[1]);

			std::vector<std::shared_ptr<AstNode>> args;
			if (auto argList = std::dynamic_pointer_cast<CommonNode>(children[3]))
			{
				if (argList->type == AstNodeType::BLOCK)
				{
					args = argList->children;
				}
			}
			result = std::make_shared<TsopInitNode>(id, stateNode->schemaName, stateNode->stateName, args);
			break;
		}

		case AstNodeType::IMPL_DECL: {
			auto stateNode = std::dynamic_pointer_cast<ImplStateNode>(children[1]);

			std::vector<std::shared_ptr<MethodDeclNode>> methods;
			if (auto block = std::dynamic_pointer_cast<CommonNode>(children[3]))
			{
				if (block->type == AstNodeType::BLOCK)
				{
					for (auto& child : block->children)
					{
						if (const auto mElem = std::dynamic_pointer_cast<MethodDeclNode>(child))
							methods.push_back(mElem);
					}
				}
			}
			result = std::make_shared<ImplDeclNode>(id, stateNode->schemaName, stateNode->stateName, methods);
			break;
		}

		case AstNodeType::METHOD_DECL: {
			bool isPublic = GetToken(children[0]).value == "public";
			std::string name = GetToken(children[2]).value;

			std::vector<std::shared_ptr<VarDeclNode>> params;
			if (auto paramList = std::dynamic_pointer_cast<CommonNode>(children[4]))
			{
				if (paramList->type == AstNodeType::BLOCK)
				{
					for (auto& child : paramList->children)
					{
						if (const auto pElem = std::dynamic_pointer_cast<VarDeclNode>(child))
							params.push_back(pElem);
					}
				}
			}

			auto targetState = std::dynamic_pointer_cast<ImplStateNode>(children[7]);

			auto body = children[9];

			result = std::make_shared<MethodDeclNode>(id, isPublic, name, params, targetState, body);
			break;
		}

		case AstNodeType::METHOD_CALL: {
			auto object = children[0];
			std::string methodName = GetToken(children[2]).value;

			std::vector<std::shared_ptr<AstNode>> args;
			if (auto argList = std::dynamic_pointer_cast<CommonNode>(children[4]))
			{
				if (argList->type == AstNodeType::BLOCK)
				{
					args = argList->children;
				}
			}

			result = std::make_shared<MethodCallNode>(id, object, methodName, args);
			break;
		}

		case AstNodeType::ENUM_CASE: {
			const auto caseName = GetToken(children[1]).value;
			auto valueNode = children[3];

			result = std::make_shared<EnumCaseNode>(id, caseName, valueNode);
			break;
		}

		case AstNodeType::ENUM_DECL: {
			const auto enumName = GetToken(children[1]).value;
			const auto backingType = ExtractTypeInfo(children[3]).type;

			std::vector<std::shared_ptr<EnumCaseNode>> enumCases;

			auto casesList = UnwrapList(children[5]);
			for (auto& child : casesList)
			{
				if (auto caseNode = std::dynamic_pointer_cast<EnumCaseNode>(child))
				{
					enumCases.push_back(caseNode);
				}
			}

			result = std::make_shared<EnumDeclNode>(id, enumName, backingType, enumCases);
			break;
		}

		case AstNodeType::MATCH_CASE: {
			result = std::make_shared<MatchCaseNode>(id, children[0], children[2]);
			break;
		}

		case AstNodeType::MATCH_EXPR: {
			auto targetExpr = children[1];
			std::vector<std::shared_ptr<MatchCaseNode>> matchCases;

			auto casesList = UnwrapList(children[3]);
			for (auto& child : casesList)
			{
				if (auto caseNode = std::dynamic_pointer_cast<MatchCaseNode>(child))
				{
					matchCases.push_back(caseNode);
				}
			}
			result = std::make_shared<MatchNode>(id, targetExpr, matchCases);
			break;
		}

		default:
			throw std::runtime_error("AstFactory Error: AstNodeType is not being processed " + std::to_string(static_cast<int>(type)));
		}
	}
	catch (const std::exception& e)
	{
		throw std::runtime_error("Crash while building rule type [" + std::to_string(static_cast<int>(type)) + "]: " + e.what());
	}
	if (result)
	{
		result->location = ExtractLocation(children);
	}
	return result;
}

Token GetToken(const std::shared_ptr<AstNode>& node)
{
	auto term = std::dynamic_pointer_cast<TerminalNode>(node);
	if (term)
	{
		return term->token;
	}

	if (const auto common = std::dynamic_pointer_cast<CommonNode>(node))
	{
		throw std::runtime_error("AST Builder error: Expected TerminalNode, but got CommonNode with type " + std::to_string(static_cast<int>(common->type)));
	}
	throw std::runtime_error("AST Builder error: Expected TerminalNode, but got some complex AST node!");
}

std::shared_ptr<AstNode> AppendToList(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	auto block = std::make_shared<CommonNode>(id, AstNodeType::BLOCK, std::vector<std::shared_ptr<AstNode>>{});

	block->location = ExtractLocation(children);
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

TypeInfo ExtractTypeInfo(const std::shared_ptr<AstNode>& typeNode)
{
	if (!typeNode)
	{
		return { DataType::UNKNOWN, 0, "" };
	}

	switch (typeNode->type)
	{
	case AstNodeType::TYPE_INT:
		return { DataType::INT, 0, "" };
	case AstNodeType::TYPE_FLOAT:
		return { DataType::FLOAT, 0, "" };
	case AstNodeType::TYPE_BOOL:
		return { DataType::BOOL, 0, "" };
	case AstNodeType::TYPE_STRING:
		return { DataType::STRING, 0, "" };
	case AstNodeType::TYPE_VOID:
		return { DataType::VOID, 0, "" };

	case AstNodeType::TYPE_CUSTOM: {
		if (const auto commonNode = std::dynamic_pointer_cast<CommonNode>(typeNode))
		{
			if (!commonNode->children.empty())
			{
				const auto name = GetToken(commonNode->children[0]).value;
				return { DataType::INSTANCE, 0, name };
			}
		}
		throw std::runtime_error("AST Factory error: Invalid TYPE_CUSTOM node");
	}

	case AstNodeType::TYPE_ARRAY: {
		if (const auto commonNode = std::dynamic_pointer_cast<CommonNode>(typeNode))
		{
			if (!commonNode->children.empty())
			{
				auto baseType = ExtractTypeInfo(commonNode->children[0]);
				baseType.depth += 1;
				return baseType;
			}
		}
		throw std::runtime_error("AST Factory error: Invalid TYPE_ARRAY node");
	}

	default:
		return { DataType::UNKNOWN, 0, "" };
	}
}

SourceLocation ExtractLocation(const std::vector<std::shared_ptr<AstNode>>& children)
{
	if (!children.empty() && children[0])
	{
		return children[0]->location;
	}
	return {};
}
