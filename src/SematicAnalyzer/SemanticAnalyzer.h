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
	bool isMoved;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(SymbolInfo, type, arrayDepth, structName, isConst, isMoved);
};

struct MethodSignature
{
	std::string returnState;
	std::vector<DataType> paramTypes;
	std::vector<std::string> paramStructNames;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(MethodSignature, returnState, paramTypes, paramStructNames);
};

struct StateSignature
{
	std::unordered_map<std::string, MethodSignature> transitions;
	size_t fieldCount = 0;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(StateSignature, fieldCount, transitions);
};

struct SchemaSignature
{
	std::unordered_map<std::string, StateSignature> states;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(SchemaSignature, states);
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

	void Visit(EnumCaseNode& node) override;

	void Visit(EnumDeclNode& node) override;

	void Visit(MatchNode& node) override;

	void Visit(MatchCaseNode& node) override;

	void Visit(BreakNode& node) override;

	void Visit(ContinueNode& node) override;

	void ExportModuleMetadata(const std::string& filepath) const;

	void ImportModuleMetadata(const std::string& filepath);

private:
	void EnterScope();

	void ExitScope();

	void EnterSchemaContext(const ImplNode& node);

	void DropSchemaContext();

	void ValidateTypeExists(DataType type, const std::string& structName, const SourceLocation& loc) const;

	void AcceptNode(const std::shared_ptr<AstNode>& node);

	void RegisterNativeLib();

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
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(StructSignature, fieldCount, fieldTypes);
	};

	struct EnumSignature
	{
		DataType backingType;
		std::unordered_map<std::string, std::string> cases;
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(EnumSignature, backingType, cases)
	};

	std::unordered_map<std::string, StructSignature> m_structSignatures;
	std::unordered_map<std::string, EnumSignature> m_enums;

	std::unordered_map<std::string, SchemaSignature> m_schemas;
	std::vector<std::unordered_map<std::string, SymbolInfo>> m_symbolTable;
	std::unordered_map<std::string, SymbolInfo> m_functions;
	std::string m_currentImplSchema;
	std::string m_currentImplState;
	int m_loopDepth = 0;

	SymbolInfo m_currentFunctionSignature{
		DataType::UNKNOWN,
		0, "",
		false
	};

	std::vector<std::unordered_map<std::string, TsopType>> m_tsopVars;
	std::unordered_map<const AstNode*, TsopType> m_nodeTypes;
};
