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
};