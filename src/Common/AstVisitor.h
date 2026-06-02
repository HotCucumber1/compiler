#pragma once

struct TerminalNode;
struct CommonNode;
struct BinaryOpNode;
struct UnaryOpNode;
struct VarDeclNode;
struct LiteralNode;
struct VarAccessNode;
struct AssignmentNode;
struct IfNode;
struct WhileNode;
struct ForNode;
struct CallNode;
struct FunctionDeclNode;
struct StructDeclNode;
struct StructInitNode;
struct MemberAccessNode;
struct MemberAssignmentNode;
struct ArrayLiteralNode;
struct IndexAccessNode;
struct IndexAssignmentNode;
struct ReturnNode;
struct ImportNode;
struct SchemaFuncNode;
struct SchemaStateNode;
struct SchemaDeclNode;
struct ImplDeclNode;
struct MethodDeclNode;
struct ImplStateNode;
struct TsopInitNode;
struct MethodCallNode;

class AstVisitor
{
public:
	virtual ~AstVisitor() = default;

	virtual void Visit(TerminalNode& node) = 0;

	virtual void Visit(CommonNode& node) = 0;

	virtual void Visit(BinaryOpNode& node) = 0;

	virtual void Visit(UnaryOpNode& node) = 0;

	virtual void Visit(VarDeclNode& node) = 0;

	virtual void Visit(LiteralNode& node) = 0;

	virtual void Visit(VarAccessNode& node) = 0;

	virtual void Visit(AssignmentNode& node) = 0;

	virtual void Visit(IfNode& node) = 0;

	virtual void Visit(WhileNode& node) = 0;

	virtual void Visit(ForNode& node) = 0;

	virtual void Visit(CallNode& node) = 0;

	virtual void Visit(FunctionDeclNode& node) = 0;

	virtual void Visit(StructDeclNode& node) = 0;

	virtual void Visit(StructInitNode& node) = 0;

	virtual void Visit(MemberAccessNode& node) = 0;

	virtual void Visit(MemberAssignmentNode& node) = 0;

	virtual void Visit(ArrayLiteralNode& node) = 0;

	virtual void Visit(IndexAccessNode& node) = 0;

	virtual void Visit(IndexAssignmentNode& node) = 0;

	virtual void Visit(ReturnNode& node) = 0;

	virtual void Visit(ImportNode& node) = 0;

	virtual void Visit(SchemaFuncNode& node) = 0;

	virtual void Visit(SchemaStateNode& node) = 0;

	virtual void Visit(SchemaDeclNode& node) = 0;

	virtual void Visit(ImplDeclNode& node) = 0;

	virtual void Visit(MethodDeclNode& node) = 0;

	virtual void Visit(ImplStateNode& node) = 0;

	virtual void Visit(TsopInitNode& node) = 0;

	virtual void Visit(MethodCallNode& node) = 0;
};
