#pragma once
#include "../../Common/Token.h"
#include "AstVisitor.h"

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
	STRUCT_INIT
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
};

using SymbolId = int32_t;

struct AstNode
{
	SymbolId symbolId;
	AstNodeType type = AstNodeType::UNDEFINED;

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

	VarDeclNode(
		const SymbolId id,
		std::string name,
		std::shared_ptr<AstNode> e,
		const bool c)
		: AstNode(id, AstNodeType::VAR_DECL)
		, varName(std::move(name))
		, expr(std::move(e))
		, isConst(c)
	{
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
	std::vector<std::string> parameters;
	std::shared_ptr<AstNode> body;

	FunctionDeclNode(const SymbolId id, std::string n, std::vector<std::string> p, std::shared_ptr<AstNode> b)
		: AstNode(id, AstNodeType::FUNC_DECL)
		, name(std::move(n))
		, parameters(std::move(p))
		, body(std::move(b))
	{
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
	std::string structName;
	std::vector<std::shared_ptr<AstNode>> args;

	StructInitNode(const SymbolId id, std::string name, std::vector<std::shared_ptr<AstNode>> args)
		: AstNode(id, AstNodeType::STRUCT_INIT)
		, structName(std::move(name))
		, args(std::move(args))
	{
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
