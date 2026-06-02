#pragma once
#include "../Common/AstVisitor.h"
#include "../Common/DataType.h"
#include "../Common/Node.h"

struct SymbolInfo
{
	DataType type;
	int arrayDepth;
	std::string structName;
	bool isConst;
};

struct StateSignature
{
	std::unordered_map<std::string, std::string> transitions;
	size_t fieldCount = 0;
};

struct SchemaSignature
{
	std::unordered_map<std::string, StateSignature> states;
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

	void Visit(ImplDeclNode& node) override;

	void Visit(MethodDeclNode& node) override;

	void Visit(TsopInitNode& node) override;

	void Visit(ImplStateNode& node) override;

	void Visit(MethodCallNode& node) override;

private:
	void EnterScope();

	void ExitScope();

	void EnterSchemaContext(const ImplNode& node);

	void DropSchemaContext();

	void ValidateTypeExists(DataType type, const std::string& structName, const SourceLocation& loc) const;

	void AcceptNode(const std::shared_ptr<AstNode>& node);

private:
	struct TsopType
	{
		std::string schema;
		std::string state;
	};

	struct StructSignature
	{
		std::unordered_map<std::string, SymbolInfo> fieldTypes;
		size_t fieldCount = 0;
	};

	std::unordered_map<std::string, StructSignature> m_structSignatures;

	std::unordered_map<std::string, SchemaSignature> m_schemas;
	std::vector<std::unordered_map<std::string, SymbolInfo>> m_symbolTable;
	std::unordered_map<std::string, SymbolInfo> m_functions;
	std::string m_currentImplSchema;
	std::string m_currentImplState;

	SymbolInfo m_currentFunctionSignature{
		DataType::UNKNOWN,
		0, "",
		false
	};

	std::vector<std::unordered_map<std::string, TsopType>> m_tsopVars;
	std::unordered_map<const AstNode*, TsopType> m_nodeTypes;
};
