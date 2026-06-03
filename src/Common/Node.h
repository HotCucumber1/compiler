#pragma once
#include "../ErrorHandler/ErrorHandler.h"
#include "AstVisitor.h"
#include "DataType.h"
#include "Token.h"

#include <memory>
#include <utility>
#include <vector>

enum class AstNodeType : int
{
	UNDEFINED,
	PROGRAM,
	BLOCK,
	VAR_DECL,
	CONST_DECL,
	FUNC_DECL,
	IF_STATEMENT,
	WHILE_STATEMENT,
	FOR_STATEMENT,
	ASSIGNMENT,
	EXPRESSION,
	BINARY_OP,
	UNARY_OP,
	LITERAL,
	TERMINAL,
	CALL,
	VAR_ACCESS,
	LIST_APPEND,
	LIST_SINGLE,
	PASS_THROUGH,
	STRUCT_DECL,
	MEMBER_ASSIGNMENT,
	MEMBER_ACCESS,
	STRUCT_INIT,
	ARRAY_LITERAL,
	INDEX_ACCESS,
	INDEX_ASSIGN,
	RETURN_STATEMENT,
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_BOOL,
	TYPE_STRING,
	TYPE_VOID,
	TYPE_CUSTOM,
	TYPE_ARRAY,
	IMPORT_DECL,
	USE_DECL,
	SCHEMA_DECL,
	SCHEMA_STATE,
	SCHEMA_FUNC,
	IMPL_DECL,
	METHOD_DECL,
	TSOP_INIT,
	IMPL_STATE_TYPE,
	METHOD_CALL,
	ENUM_CASE,
	ENUM_DECL,
};

static const std::unordered_map<std::string, AstNodeType> AST_NODE_TYPES = {
	{ "UNDEFINED", AstNodeType::UNDEFINED },
	{ "PROGRAM", AstNodeType::PROGRAM },
	{ "BLOCK", AstNodeType::BLOCK },
	{ "VAR_DECL", AstNodeType::VAR_DECL },
	{ "IF_STATEMENT", AstNodeType::IF_STATEMENT },
	{ "WHILE_STATEMENT", AstNodeType::WHILE_STATEMENT },
	{ "FOR_STATEMENT", AstNodeType::FOR_STATEMENT },
	{ "ASSIGNMENT", AstNodeType::ASSIGNMENT },
	{ "EXPRESSION", AstNodeType::EXPRESSION },
	{ "BINARY_OP", AstNodeType::BINARY_OP },
	{ "UNARY_OP", AstNodeType::UNARY_OP },
	{ "LITERAL", AstNodeType::LITERAL },
	{ "TERMINAL", AstNodeType::TERMINAL },
	{ "CALL", AstNodeType::CALL },
	{ "VAR_ACCESS", AstNodeType::VAR_ACCESS },
	{ "LIST_APPEND", AstNodeType::LIST_APPEND },
	{ "LIST_SINGLE", AstNodeType::LIST_SINGLE },
	{ "PASS_THROUGH", AstNodeType::PASS_THROUGH },
	{ "FUNC_DECL", AstNodeType::FUNC_DECL },
	{ "STRUCT_DECL", AstNodeType::STRUCT_DECL },
	{ "MEMBER_ASSIGNMENT", AstNodeType::MEMBER_ASSIGNMENT },
	{ "MEMBER_ACCESS", AstNodeType::MEMBER_ACCESS },
	{ "STRUCT_INIT", AstNodeType::STRUCT_INIT },
	{ "ARRAY_LITERAL", AstNodeType::ARRAY_LITERAL },
	{ "INDEX_ACCESS", AstNodeType::INDEX_ACCESS },
	{ "INDEX_ASSIGN", AstNodeType::INDEX_ASSIGN },
	{ "RETURN_STATEMENT", AstNodeType::RETURN_STATEMENT },
	{ "TYPE_INT", AstNodeType::TYPE_INT },
	{ "TYPE_FLOAT", AstNodeType::TYPE_FLOAT },
	{ "TYPE_BOOL", AstNodeType::TYPE_BOOL },
	{ "TYPE_STRING", AstNodeType::TYPE_STRING },
	{ "TYPE_VOID", AstNodeType::TYPE_VOID },
	{ "TYPE_CUSTOM", AstNodeType::TYPE_CUSTOM },
	{ "TYPE_ARRAY", AstNodeType::TYPE_ARRAY },
	{ "IMPORT_DECL", AstNodeType::IMPORT_DECL },
	{ "USE_DECL", AstNodeType::USE_DECL },
	{ "SCHEMA_DECL", AstNodeType::SCHEMA_DECL },
	{ "SCHEMA_STATE", AstNodeType::SCHEMA_STATE },
	{ "SCHEMA_FUNC", AstNodeType::SCHEMA_FUNC },
	{ "IMPL_DECL", AstNodeType::IMPL_DECL },
	{ "METHOD_DECL", AstNodeType::METHOD_DECL },
	{ "TSOP_INIT", AstNodeType::TSOP_INIT },
	{ "IMPL_STATE_TYPE", AstNodeType::IMPL_STATE_TYPE },
	{ "METHOD_CALL", AstNodeType::METHOD_CALL },
	{ "ENUM_CASE", AstNodeType::ENUM_CASE },
	{ "ENUM_DECL", AstNodeType::ENUM_DECL },
};

using SymbolId = int32_t;

struct AstNode
{
	SymbolId symbolId;
	AstNodeType type = AstNodeType::UNDEFINED;
	DataType evaluatedType = DataType::UNKNOWN;
	int arrayDepth = 0;
	std::string structName;
	SourceLocation location;

	AstNode(const SymbolId id, const AstNodeType t)
		: symbolId(id)
		, type(t)
	{
	}

	virtual ~AstNode() = default;

	virtual void Accept(AstVisitor& visitor);
};

struct TerminalNode final : AstNode
{
	Token token;
	TerminalNode(const SymbolId id, Token t)
		: AstNode(id, AstNodeType::TERMINAL)
		, token(std::move(t))
	{
		this->location = {
			token.file,
			token.line,
			token.pos
		};
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct CommonNode final : AstNode
{
	std::vector<std::shared_ptr<AstNode>> children;

	CommonNode(const SymbolId id, const AstNodeType t, std::vector<std::shared_ptr<AstNode>> c)
		: AstNode(id, t)
		, children(std::move(c))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct BinaryOpNode final : AstNode
{
	std::shared_ptr<AstNode> left;
	Token op; // TODO не факт, что токен
	std::shared_ptr<AstNode> right;

	BinaryOpNode(
		const SymbolId id,
		std::shared_ptr<AstNode> l,
		Token op,
		std::shared_ptr<AstNode> r)
		: AstNode(id, AstNodeType::BINARY_OP)
		, left(std::move(l))
		, op(std::move(op))
		, right(std::move(r))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct UnaryOpNode final : AstNode
{
	Token op;
	std::shared_ptr<AstNode> operand;

	UnaryOpNode(
		const SymbolId id,
		Token o,
		std::shared_ptr<AstNode> opnd)
		: AstNode(id, AstNodeType::UNARY_OP)
		, op(std::move(o))
		, operand(std::move(opnd))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct VarDeclNode final : AstNode
{
	std::string varName;
	std::shared_ptr<AstNode> expr;
	bool isConst;
	DataType varType = DataType::UNKNOWN;

	VarDeclNode(
		const SymbolId id,
		std::string name,
		std::shared_ptr<AstNode> e,
		const bool isConst,
		const int depth,
		const DataType& varType,
		std::string sName = "")
		: AstNode(id, AstNodeType::VAR_DECL)
		, varName(std::move(name))
		, expr(std::move(e))
		, isConst(isConst)
		, varType(varType)
	{
		this->arrayDepth = depth;
		this->structName = std::move(sName);
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};
struct LiteralNode final : AstNode
{
	Token token;
	LiteralNode(const SymbolId id, Token t)
		: AstNode(id, AstNodeType::LITERAL)
		, token(std::move(t))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct VarAccessNode final : AstNode
{
	std::string varName;
	bool isEnumName = false;
	VarAccessNode(const SymbolId id, std::string name)
		: AstNode(id, AstNodeType::VAR_ACCESS)
		, varName(std::move(name))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct AssignmentNode final : AstNode
{
	std::string varName;
	std::shared_ptr<AstNode> expr;

	AssignmentNode(
		const SymbolId id,
		std::string name,
		std::shared_ptr<AstNode> e)
		: AstNode(id, AstNodeType::ASSIGNMENT)
		, varName(std::move(name))
		, expr(std::move(e))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct IfNode final : AstNode
{
	std::shared_ptr<AstNode> condition;
	std::shared_ptr<AstNode> thenBranch;
	std::shared_ptr<AstNode> elseBranch;

	IfNode(
		const SymbolId id,
		std::shared_ptr<AstNode> cond,
		std::shared_ptr<AstNode> thenBranch,
		std::shared_ptr<AstNode> elseBranch)
		: AstNode(id, AstNodeType::IF_STATEMENT)
		, condition(std::move(cond))
		, thenBranch(std::move(thenBranch))
		, elseBranch(std::move(elseBranch))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct WhileNode final : AstNode
{
	std::shared_ptr<AstNode> condition;
	std::shared_ptr<AstNode> body;

	WhileNode(
		const SymbolId id,
		std::shared_ptr<AstNode> cond,
		std::shared_ptr<AstNode> b)
		: AstNode(id, AstNodeType::WHILE_STATEMENT)
		, condition(std::move(cond))
		, body(std::move(b))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct ForNode final : AstNode
{
	std::shared_ptr<AstNode> init;
	std::shared_ptr<AstNode> condition;
	std::shared_ptr<AstNode> increment;
	std::shared_ptr<AstNode> body;

	ForNode(
		const SymbolId id,
		std::shared_ptr<AstNode> i,
		std::shared_ptr<AstNode> cond,
		std::shared_ptr<AstNode> inc,
		std::shared_ptr<AstNode> body)
		: AstNode(id, AstNodeType::FOR_STATEMENT)
		, init(std::move(i))
		, condition(std::move(cond))
		, increment(std::move(inc))
		, body(std::move(body))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct CallNode final : AstNode
{
	std::string funcName;
	std::vector<std::shared_ptr<AstNode>> args;

	CallNode(
		const SymbolId id,
		std::string name,
		std::vector<std::shared_ptr<AstNode>> a)
		: AstNode(id, AstNodeType::CALL)
		, funcName(std::move(name))
		, args(std::move(a))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct FunctionDeclNode final : AstNode
{
	std::string name;
	std::vector<std::shared_ptr<VarDeclNode>> parameters;
	std::vector<std::shared_ptr<VarDeclNode>> captures;
	std::shared_ptr<AstNode> body;

	DataType returnType;

	FunctionDeclNode(
		const SymbolId id,
		std::string n,
		std::vector<std::shared_ptr<VarDeclNode>> p,
		std::vector<std::shared_ptr<VarDeclNode>> c,
		std::shared_ptr<AstNode> b,
		const DataType rType = DataType::UNKNOWN,
		const int arrDepth = 0,
		std::string sName = "")
		: AstNode(id, AstNodeType::FUNC_DECL)
		, name(std::move(n))
		, parameters(std::move(p))
		, captures(std::move(c))
		, body(std::move(b))
		, returnType(rType)
	{
		this->arrayDepth = arrDepth;
		this->structName = std::move(sName);
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct StructDeclNode final : AstNode
{
	std::string name;
	std::vector<std::shared_ptr<AstNode>> fields;

	StructDeclNode(const SymbolId id, std::string name, std::vector<std::shared_ptr<AstNode>> fields)
		: AstNode(id, AstNodeType::STRUCT_DECL)
		, name(std::move(name))
		, fields(std::move(fields))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct StructInitNode final : AstNode
{
	std::vector<std::shared_ptr<AstNode>> args;

	StructInitNode(const SymbolId id, std::string name, std::vector<std::shared_ptr<AstNode>> args)
		: AstNode(id, AstNodeType::STRUCT_INIT)
		, args(std::move(args))
	{
		this->structName = std::move(name);
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct MemberAccessNode final : AstNode
{
	std::shared_ptr<AstNode> object;
	std::string fieldName;

	bool isEnumCase = false;
	std::string enumLiteralValue;
	DataType enumBackingType = DataType::UNKNOWN;

	MemberAccessNode(const SymbolId id, std::shared_ptr<AstNode> obj, std::string field)
		: AstNode(id, AstNodeType::MEMBER_ACCESS)
		, object(std::move(obj))
		, fieldName(std::move(field))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct MemberAssignmentNode final : AstNode
{
	std::shared_ptr<AstNode> object;
	std::string fieldName;
	std::shared_ptr<AstNode> expression;

	MemberAssignmentNode(const SymbolId id, std::shared_ptr<AstNode> obj, std::string field, std::shared_ptr<AstNode> expr)
		: AstNode(id, AstNodeType::MEMBER_ASSIGNMENT)
		, object(std::move(obj))
		, fieldName(std::move(field))
		, expression(std::move(expr))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct ArrayLiteralNode final : AstNode
{
	std::vector<std::shared_ptr<AstNode>> elements;
	ArrayLiteralNode(const SymbolId id, std::vector<std::shared_ptr<AstNode>> elems)
		: AstNode(id, AstNodeType::ARRAY_LITERAL)
		, elements(std::move(elems))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct IndexAccessNode final : AstNode
{
	std::shared_ptr<AstNode> target;
	std::shared_ptr<AstNode> index;
	IndexAccessNode(const SymbolId id, std::shared_ptr<AstNode> tgt, std::shared_ptr<AstNode> idx)
		: AstNode(id, AstNodeType::INDEX_ACCESS)
		, target(std::move(tgt))
		, index(std::move(idx))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct IndexAssignmentNode final : AstNode
{
	std::shared_ptr<AstNode> target;
	std::shared_ptr<AstNode> index;
	std::shared_ptr<AstNode> value;
	IndexAssignmentNode(const SymbolId id, std::shared_ptr<AstNode> tgt, std::shared_ptr<AstNode> idx, std::shared_ptr<AstNode> val)
		: AstNode(id, AstNodeType::INDEX_ASSIGN)
		, target(std::move(tgt))
		, index(std::move(idx))
		, value(std::move(val))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct ReturnNode final : AstNode
{
	std::shared_ptr<AstNode> expr;

	explicit ReturnNode(const SymbolId id, std::shared_ptr<AstNode> e = nullptr)
		: AstNode(id, AstNodeType::RETURN_STATEMENT)
		, expr(std::move(e))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct ImportNode final : AstNode
{
	std::string filePath;

	ImportNode(const SymbolId id, std::string path)
		: AstNode(id, AstNodeType::IMPORT_DECL)
		, filePath(std::move(path))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct SchemaFuncNode final : AstNode
{
	std::string name;
	std::vector<std::shared_ptr<VarDeclNode>> parameters;
	std::string returnState;

	SchemaFuncNode(
		const SymbolId id,
		std::string n,
		std::vector<std::shared_ptr<VarDeclNode>> params,
		std::string retState)
		: AstNode(id, AstNodeType::SCHEMA_FUNC)
		, name(std::move(n))
		, parameters(std::move(params))
		, returnState(std::move(retState))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct SchemaStateNode final : AstNode
{
	std::string name;
	std::vector<std::shared_ptr<VarDeclNode>> fields;
	std::vector<std::shared_ptr<SchemaFuncNode>> methods;

	SchemaStateNode(
		const SymbolId id,
		std::string n,
		std::vector<std::shared_ptr<VarDeclNode>> f,
		std::vector<std::shared_ptr<SchemaFuncNode>> m)
		: AstNode(id, AstNodeType::SCHEMA_STATE)
		, name(std::move(n))
		, fields(std::move(f))
		, methods(std::move(m))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct SchemaDeclNode final : AstNode
{
	std::string name;
	std::vector<std::shared_ptr<SchemaStateNode>> states;

	SchemaDeclNode(
		const SymbolId id,
		std::string n,
		std::vector<std::shared_ptr<SchemaStateNode>> s)
		: AstNode(id, AstNodeType::SCHEMA_DECL)
		, name(std::move(n))
		, states(std::move(s))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct MethodDeclNode final : AstNode
{
	bool isPublic;
	std::string name;
	std::vector<std::shared_ptr<VarDeclNode>> parameters;
	std::shared_ptr<ImplStateNode> targetState;
	std::shared_ptr<AstNode> body;

	MethodDeclNode(
		const SymbolId id,
		const bool pub,
		std::string n,
		std::vector<std::shared_ptr<VarDeclNode>> params,
		std::shared_ptr<ImplStateNode> target,
		std::shared_ptr<AstNode> b)
		: AstNode(id, AstNodeType::METHOD_DECL)
		, isPublic(pub)
		, name(std::move(n))
		, parameters(std::move(params))
		, targetState(std::move(target))
		, body(std::move(b))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct ImplNode : AstNode
{
	std::string schemaName;
	std::string stateName;

	ImplNode(const SymbolId id, const AstNodeType type, std::string schema, std::string state)
		: AstNode(id, type)
		, schemaName(std::move(schema))
		, stateName(std::move(state))
	{
	}
};

struct ImplDeclNode final : ImplNode
{
	std::vector<std::shared_ptr<MethodDeclNode>> methods;

	ImplDeclNode(
		const SymbolId id,
		const std::string& sName,
		const std::string& stName,
		std::vector<std::shared_ptr<MethodDeclNode>> methods)
		: ImplNode(id, AstNodeType::IMPL_DECL, sName, stName)
		, methods(std::move(methods))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct ImplStateNode final : ImplNode
{
	ImplStateNode(
		const SymbolId id,
		const std::string& schema,
		const std::string& state)
		: ImplNode(id, AstNodeType::IMPL_STATE_TYPE, schema, state)
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct TsopInitNode final : ImplNode
{
	std::vector<std::shared_ptr<AstNode>> args;

	TsopInitNode(
		const SymbolId id,
		const std::string& schema,
		const std::string& state,
		std::vector<std::shared_ptr<AstNode>> a)
		: ImplNode(id, AstNodeType::TSOP_INIT, schema, state)
		, args(std::move(a))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct MethodCallNode final : AstNode
{
	std::shared_ptr<AstNode> object;
	std::string methodName;
	std::vector<std::shared_ptr<AstNode>> args;
	std::string mangledFuncName;

	MethodCallNode(
		const SymbolId id,
		std::shared_ptr<AstNode> obj,
		std::string mName,
		std::vector<std::shared_ptr<AstNode>> a)
		: AstNode(id, AstNodeType::METHOD_CALL)
		, object(std::move(obj))
		, methodName(std::move(mName))
		, args(std::move(a))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct EnumCaseNode final : AstNode
{
	std::string name;
	std::shared_ptr<AstNode> value;

	EnumCaseNode(
		const SymbolId id,
		std::string caseName,
		std::shared_ptr<AstNode> val)
		: AstNode(id, AstNodeType::ENUM_CASE)
		, name(std::move(caseName))
		, value(std::move(val))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};

struct EnumDeclNode final : AstNode
{
	std::string name;
	DataType backingType;
	std::vector<std::shared_ptr<EnumCaseNode>> cases;

	EnumDeclNode(
		const SymbolId id,
		std::string enumName,
		const DataType type,
		std::vector<std::shared_ptr<EnumCaseNode>> c)
		: AstNode(id, AstNodeType::ENUM_DECL)
		, name(std::move(enumName))
		, backingType(type)
		, cases(std::move(c))
	{
	}

	void Accept(AstVisitor& visitor) override
	{
		visitor.Visit(*this);
	}
};
