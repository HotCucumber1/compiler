#pragma once
#include "../Parser/Common/AstVisitor.h"
#include "../Parser/Common/DataType.h"
#include "../Parser/Common/Node.h"

struct SymbolInfo
{
	DataType type;
	int arrayDepth;
	std::string structName;
	bool isConst;
};

class SemanticAnalyzer final : public AstVisitor
{
public:
	void Analyze(const std::shared_ptr<AstNode>& root);

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

	void Visit(ImportNode& node) override;

	void Visit(SchemaFuncNode& node) override;

	void Visit(SchemaStateNode& node) override;

	void Visit(SchemaDeclNode& node) override;


private:
	void EnterScope();

	void ExitScope();

private:
	std::vector<std::unordered_map<std::string, SymbolInfo>> m_symbolTable;
	std::unordered_map<std::string, SymbolInfo> m_functions;
	SymbolInfo m_currentFunctionSignature{
		DataType::UNKNOWN,
		0, "",
		false
	};
};
