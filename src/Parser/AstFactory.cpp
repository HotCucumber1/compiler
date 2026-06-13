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

std::shared_ptr<AstNode> BuildBasicType(AstNodeType type, SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildProgram(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildVarDecl(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildAssignment(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildBinaryOp(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildUnaryOp(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildLiteral(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildVarAccess(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildIfStatement(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildWhileStatement(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildForStatement(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildCall(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildFuncDecl(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildStructDecl(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildStructInit(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildMemberAccess(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildMemberAssignment(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildArrayLiteral(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildIndexAccess(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildIndexAssignment(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildReturnStatement(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildImportDecl(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildSchemaFunc(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildSchemaState(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildSchemaDecl(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildImplStateType(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildTsopInit(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildImplDecl(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildMethodDecl(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildMethodCall(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildEnumCase(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildEnumDecl(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildMatchCase(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildMatchExpr(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildPassThrough(const std::vector<std::shared_ptr<AstNode>>& children);
std::shared_ptr<AstNode> BuildContinue(SymbolId id);
std::shared_ptr<AstNode> BuildBreak(SymbolId id);

std::shared_ptr<AstNode> AstFactory::CreateNode(
	AstNodeType type,
	const SymbolId id,
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
		case AstNodeType::TYPE_TSOP:
		case AstNodeType::TYPE_FUNC:
			result = BuildBasicType(type, id, children);
			break;
		case AstNodeType::PROGRAM:
			result = BuildProgram(id, children);
			break;
		case AstNodeType::VAR_DECL:
		case AstNodeType::CONST_DECL:
			result = BuildVarDecl(id, children);
			break;
		case AstNodeType::ASSIGNMENT:
			result = BuildAssignment(id, children);
			break;
		case AstNodeType::BINARY_OP:
			result = BuildBinaryOp(id, children);
			break;
		case AstNodeType::UNARY_OP:
			result = BuildUnaryOp(id, children);
			break;
		case AstNodeType::LITERAL:
			result = BuildLiteral(id, children);
			break;
		case AstNodeType::VAR_ACCESS:
			result = BuildVarAccess(id, children);
			break;
		case AstNodeType::IF_STATEMENT:
			result = BuildIfStatement(id, children);
			break;
		case AstNodeType::WHILE_STATEMENT:
			result = BuildWhileStatement(id, children);
			break;
		case AstNodeType::FOR_STATEMENT:
			result = BuildForStatement(id, children);
			break;
		case AstNodeType::CALL:
			result = BuildCall(id, children);
			break;
		case AstNodeType::FUNC_DECL:
			result = BuildFuncDecl(id, children);
			break;
		case AstNodeType::STRUCT_DECL:
			result = BuildStructDecl(id, children);
			break;
		case AstNodeType::STRUCT_INIT:
			result = BuildStructInit(id, children);
			break;
		case AstNodeType::MEMBER_ACCESS:
			result = BuildMemberAccess(id, children);
			break;
		case AstNodeType::MEMBER_ASSIGNMENT:
			result = BuildMemberAssignment(id, children);
			break;
		case AstNodeType::ARRAY_LITERAL:
			result = BuildArrayLiteral(id, children);
			break;
		case AstNodeType::INDEX_ACCESS:
			result = BuildIndexAccess(id, children);
			break;
		case AstNodeType::INDEX_ASSIGN:
			result = BuildIndexAssignment(id, children);
			break;
		case AstNodeType::RETURN_STATEMENT:
			result = BuildReturnStatement(id, children);
			break;
		case AstNodeType::IMPORT_DECL:
			result = BuildImportDecl(id, children);
			break;
		case AstNodeType::SCHEMA_FUNC:
			result = BuildSchemaFunc(id, children);
			break;
		case AstNodeType::SCHEMA_STATE:
			result = BuildSchemaState(id, children);
			break;
		case AstNodeType::SCHEMA_DECL:
			result = BuildSchemaDecl(id, children);
			break;
		case AstNodeType::IMPL_STATE_TYPE:
			result = BuildImplStateType(id, children);
			break;
		case AstNodeType::TSOP_INIT:
			result = BuildTsopInit(id, children);
			break;
		case AstNodeType::IMPL_DECL:
			result = BuildImplDecl(id, children);
			break;
		case AstNodeType::METHOD_DECL:
			result = BuildMethodDecl(id, children);
			break;
		case AstNodeType::METHOD_CALL:
			result = BuildMethodCall(id, children);
			break;
		case AstNodeType::ENUM_CASE:
			result = BuildEnumCase(id, children);
			break;
		case AstNodeType::ENUM_DECL:
			result = BuildEnumDecl(id, children);
			break;
		case AstNodeType::MATCH_CASE:
			result = BuildMatchCase(id, children);
			break;
		case AstNodeType::MATCH_EXPR:
			result = BuildMatchExpr(id, children);
			break;
		case AstNodeType::BREAK:
			result = BuildBreak(id);
			break;
		case AstNodeType::CONTINUE:
			result = BuildContinue(id);
			break;

		case AstNodeType::USE_DECL:
			return children[2];
		case AstNodeType::LIST_APPEND:
		case AstNodeType::LIST_SINGLE:
			return AppendToList(id, children);
		case AstNodeType::PASS_THROUGH:
			return BuildPassThrough(children);

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

std::shared_ptr<AstNode> BuildBasicType(AstNodeType type, SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	return std::make_shared<CommonNode>(id, type, children);
}

std::shared_ptr<AstNode> BuildProgram(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	if (!children.empty())
	{
		if (auto block = std::dynamic_pointer_cast<CommonNode>(children[0]))
		{
			return std::make_shared<CommonNode>(id, AstNodeType::PROGRAM, block->children);
		}
	}
	return std::make_shared<CommonNode>(id, AstNodeType::PROGRAM, children);
}

std::shared_ptr<AstNode> BuildVarDecl(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	int typeIdx = 0;
	int idIdx = 1;
	int exprIdx = 3;
	bool isConst = false;

	if (auto term = std::dynamic_pointer_cast<TerminalNode>(children[0]))
	{
		if (term->token.value == "let")
		{
			typeIdx = 1;
			idIdx = 2;
			exprIdx = 4;
		}
		else if (term->token.value == "const")
		{
			typeIdx = 1;
			idIdx = 2;
			exprIdx = 4;
			isConst = true;
		}
	}

	auto [dataType, depth, structName] = ExtractTypeInfo(children[typeIdx]);

	std::shared_ptr<AstNode> val = nullptr;
	if (children.size() > exprIdx)
	{
		val = children[exprIdx];
	}

	return std::make_shared<VarDeclNode>(id, GetToken(children[idIdx]).value, val, isConst, depth, dataType, structName);
}

std::shared_ptr<AstNode> BuildAssignment(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	const auto& leftNode = children[0];
	const auto& rightNode = children[2];

	if (auto indexAccess = std::dynamic_pointer_cast<IndexAccessNode>(leftNode))
	{
		return std::make_shared<IndexAssignmentNode>(id, indexAccess->target, indexAccess->index, rightNode);
	}
	if (auto memberAccess = std::dynamic_pointer_cast<MemberAccessNode>(leftNode))
	{
		return std::make_shared<MemberAssignmentNode>(id, memberAccess->object, memberAccess->fieldName, rightNode);
	}
	if (auto varAccess = std::dynamic_pointer_cast<VarAccessNode>(leftNode))
	{
		return std::make_shared<AssignmentNode>(id, varAccess->varName, rightNode);
	}
	return nullptr;
}

std::shared_ptr<AstNode> BuildBinaryOp(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	return std::make_shared<BinaryOpNode>(id, children[0], GetToken(children[1]), children[2]);
}

std::shared_ptr<AstNode> BuildUnaryOp(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	return std::make_shared<UnaryOpNode>(id, GetToken(children[0]), children[1]);
}

std::shared_ptr<AstNode> BuildLiteral(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	return std::make_shared<LiteralNode>(id, GetToken(children[0]));
}

std::shared_ptr<AstNode> BuildVarAccess(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	return std::make_shared<VarAccessNode>(id, GetToken(children[0]).value);
}

std::shared_ptr<AstNode> BuildIfStatement(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
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

std::shared_ptr<AstNode> BuildWhileStatement(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	return std::make_shared<WhileNode>(id, children[2], children[5]);
}

std::shared_ptr<AstNode> BuildForStatement(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	return std::make_shared<ForNode>(id, children[2], children[4], children[6], children[9]);
}

std::shared_ptr<AstNode> BuildCall(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	std::string name = GetToken(children[0]).value;
	std::vector<std::shared_ptr<AstNode>> args;
	if (const auto argList = std::dynamic_pointer_cast<CommonNode>(children[2]))
	{
		args = argList->children;
	}
	return std::make_shared<CallNode>(id, name, args);
}

std::shared_ptr<AstNode> BuildFuncDecl(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
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
				{
					captures.push_back(cElem);
				}
			}
		}
	}
	auto [dataType, depth, structName] = ExtractTypeInfo(children[7]);
	auto body = children[9];
	return std::make_shared<FunctionDeclNode>(id, name, params, captures, body, dataType, depth, structName);
}

std::shared_ptr<AstNode> BuildStructDecl(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	auto name = GetToken(children[1]).value;
	return std::make_shared<StructDeclNode>(id, name, UnwrapList(children[3]));
}

std::shared_ptr<AstNode> BuildStructInit(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	auto name = GetToken(children[0]).value;
	return std::make_shared<StructInitNode>(id, name, UnwrapList(children[2]));
}

std::shared_ptr<AstNode> BuildMemberAccess(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	return std::make_shared<MemberAccessNode>(id, children[0], GetToken(children[2]).value);
}

std::shared_ptr<AstNode> BuildMemberAssignment(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	return std::make_shared<MemberAssignmentNode>(id, children[0], GetToken(children[2]).value, children[4]);
}

std::shared_ptr<AstNode> BuildArrayLiteral(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	if (children.size() < 3)
		throw std::runtime_error("AST Factory: ARRAY_LITERAL expects at least 3 children");
	std::vector<std::shared_ptr<AstNode>> elements;
	if (const auto& argNode = children[1])
	{
		if (auto common = std::dynamic_pointer_cast<CommonNode>(argNode))
		{
			if (common->type == AstNodeType::BLOCK || common->type == AstNodeType::LIST_SINGLE || common->type == AstNodeType::LIST_APPEND)
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
	return std::make_shared<ArrayLiteralNode>(id, std::move(elements));
}

std::shared_ptr<AstNode> BuildIndexAccess(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	if (children.size() < 4)
		throw std::runtime_error("AST Factory: INDEX_ACCESS expects at least 4 children");
	return std::make_shared<IndexAccessNode>(id, children[0], children[2]);
}

std::shared_ptr<AstNode> BuildIndexAssignment(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	if (children.size() < 6)
		throw std::runtime_error("AST Factory: INDEX_ASSIGN expects at least 6 children");
	return std::make_shared<IndexAssignmentNode>(id, children[0], children[2], children[5]);
}

std::shared_ptr<AstNode> BuildReturnStatement(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	std::shared_ptr<AstNode> expr = nullptr;
	if (children.size() == 2)
	{
		expr = children[1];
	}
	return std::make_shared<ReturnNode>(id, expr);
}

std::shared_ptr<AstNode> BuildImportDecl(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	auto rawPath = GetToken(children[1]).value;
	auto cleanPath = rawPath.substr(1, rawPath.size() - 2);
	return std::make_shared<ImportNode>(id, cleanPath);
}

std::shared_ptr<AstNode> BuildSchemaFunc(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
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
	return std::make_shared<SchemaFuncNode>(id, name, params, returnState);
}

std::shared_ptr<AstNode> BuildSchemaState(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	const auto name = GetToken(children[1]).value;
	std::vector<std::shared_ptr<VarDeclNode>> fields;
	if (auto paramList = std::dynamic_pointer_cast<CommonNode>(children[3]))
	{
		if (paramList->type == AstNodeType::BLOCK)
		{
			for (auto& child : paramList->children)
			{
				if (const auto pElem = std::dynamic_pointer_cast<VarDeclNode>(child))
					fields.push_back(pElem);
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
					methods.push_back(mElem);
			}
		}
	}
	return std::make_shared<SchemaStateNode>(id, name, fields, methods);
}

std::shared_ptr<AstNode> BuildSchemaDecl(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	const auto name = GetToken(children[1]).value;
	std::vector<std::shared_ptr<SchemaStateNode>> states;
	if (auto stateList = std::dynamic_pointer_cast<CommonNode>(children[3]))
	{
		if (stateList->type == AstNodeType::BLOCK)
		{
			for (auto& child : stateList->children)
			{
				if (const auto sElem = std::dynamic_pointer_cast<SchemaStateNode>(child))
					states.push_back(sElem);
			}
		}
	}
	return std::make_shared<SchemaDeclNode>(id, name, states);
}

std::shared_ptr<AstNode> BuildImplStateType(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	const auto schema = GetToken(children[0]).value;
	const auto state = GetToken(children[2]).value;
	return std::make_shared<ImplStateNode>(id, schema, state);
}

std::shared_ptr<AstNode> BuildTsopInit(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	auto stateNode = std::dynamic_pointer_cast<ImplStateNode>(children[1]);
	std::vector<std::shared_ptr<AstNode>> args;
	if (auto argList = std::dynamic_pointer_cast<CommonNode>(children[3]))
	{
		if (argList->type == AstNodeType::BLOCK)
			args = argList->children;
	}
	return std::make_shared<TsopInitNode>(id, stateNode->schemaName, stateNode->stateName, args);
}

std::shared_ptr<AstNode> BuildImplDecl(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
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
	return std::make_shared<ImplDeclNode>(id, stateNode->schemaName, stateNode->stateName, methods);
}

std::shared_ptr<AstNode> BuildMethodDecl(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
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
	return std::make_shared<MethodDeclNode>(id, isPublic, name, params, targetState, body);
}

std::shared_ptr<AstNode> BuildMethodCall(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	auto object = children[0];
	std::string methodName = GetToken(children[2]).value;
	std::vector<std::shared_ptr<AstNode>> args;
	if (auto argList = std::dynamic_pointer_cast<CommonNode>(children[4]))
	{
		if (argList->type == AstNodeType::BLOCK)
			args = argList->children;
	}
	return std::make_shared<MethodCallNode>(id, object, methodName, args);
}

std::shared_ptr<AstNode> BuildEnumCase(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	const auto caseName = GetToken(children[1]).value;
	auto valueNode = children[3];
	return std::make_shared<EnumCaseNode>(id, caseName, valueNode);
}

std::shared_ptr<AstNode> BuildEnumDecl(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	const auto enumName = GetToken(children[1]).value;
	const auto backingType = ExtractTypeInfo(children[3]).type;
	std::vector<std::shared_ptr<EnumCaseNode>> enumCases;
	auto casesList = UnwrapList(children[5]);
	for (auto& child : casesList)
	{
		if (auto caseNode = std::dynamic_pointer_cast<EnumCaseNode>(child))
			enumCases.push_back(caseNode);
	}
	return std::make_shared<EnumDeclNode>(id, enumName, backingType, enumCases);
}

std::shared_ptr<AstNode> BuildMatchCase(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	return std::make_shared<MatchCaseNode>(id, children[0], children[2]);
}

std::shared_ptr<AstNode> BuildMatchExpr(SymbolId id, const std::vector<std::shared_ptr<AstNode>>& children)
{
	auto targetExpr = children[2];
	std::vector<std::shared_ptr<MatchCaseNode>> matchCases;
	auto casesList = UnwrapList(children[5]);
	for (auto& child : casesList)
	{
		if (auto caseNode = std::dynamic_pointer_cast<MatchCaseNode>(child))
			matchCases.push_back(caseNode);
	}
	return std::make_shared<MatchNode>(id, targetExpr, matchCases);
}

std::shared_ptr<AstNode> BuildPassThrough(const std::vector<std::shared_ptr<AstNode>>& children)
{
	if (children.empty())
		return nullptr;
	if (children.size() == 3)
	{
		if (auto term = std::dynamic_pointer_cast<TerminalNode>(children[0]))
		{
			if (term->token.value == "(")
				return children[1];
		}
	}
	return children[0];
}

std::shared_ptr<AstNode> BuildContinue(SymbolId id)
{
	return std::make_shared<ContinueNode>(id);
}

std::shared_ptr<AstNode> BuildBreak(SymbolId id)
{
	return std::make_shared<BreakNode>(id);
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

	if (children.empty())
	{
		return block;
	}

	if (children.size() == 1)
	{
		if (children[0])
		{
			auto head = UnwrapList(children[0]);
			block->children.insert(block->children.end(), head.begin(), head.end());
		}
	}
	else if (children.size() >= 2)
	{
		if (children[0])
		{
			auto head = UnwrapList(children[0]);
			block->children.insert(block->children.end(), head.begin(), head.end());
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
	case AstNodeType::TYPE_FUNC:
		return { DataType::FUNC, 0, "" };

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
	case AstNodeType::TYPE_TSOP: {
		if (const auto commonNode = std::dynamic_pointer_cast<CommonNode>(typeNode))
		{
			if (commonNode->children.size() >= 3)
			{
				const auto schemaName = GetToken(commonNode->children[0]).value;
				const auto stateName = GetToken(commonNode->children[2]).value;
				return { DataType::INSTANCE, 0, schemaName + "_" + stateName };
			}
		}
		throw std::runtime_error("AST Factory error: Invalid TYPE_TSOP node");
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

	case AstNodeType::IMPL_STATE_TYPE: {
		if (const auto implState = std::dynamic_pointer_cast<ImplStateNode>(typeNode))
		{
			return { DataType::INSTANCE, 0, implState->schemaName + "_" + implState->stateName };
		}
		throw std::runtime_error("AST Factory error: Invalid IMPL_STATE_TYPE node");
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
