#pragma once
#include "AstVisitor.h"
#include "Node.h"

class AstPrinter final : public AstVisitor
{
public:
	void PrintAst(const std::shared_ptr<AstNode>& root);

	void Visit(TerminalNode& node) override;

	void Visit(CommonNode& node) override;

	void Visit(BinaryOpNode& node) override;

	void Visit(UnaryOpNode& node) override;

	void Visit(VarDeclNode& node) override;

	void Visit(LiteralNode& node) override;

	void Visit(VarAccessNode& node) override;

	void Visit(AssignmentNode& node) override;

	void Visit(IfNode& node) override;

	void Visit(WhileNode& node) override;

	void Visit(ForNode& node) override;

	void Visit(CallNode& node) override;

	void Visit(FunctionDeclNode& node) override;

	void Visit(StructDeclNode& node) override;

	void Visit(StructInitNode& node) override;

	void Visit(MemberAccessNode& node) override;

	void Visit(MemberAssignmentNode& node) override;

	void Visit(ArrayLiteralNode& node) override;

	void Visit(IndexAccessNode& node) override;

	void Visit(IndexAssignmentNode& node) override;

	void Visit(ReturnNode& node) override;

private:
	int m_depth = 0;

	void PrintIndent() const;
};
