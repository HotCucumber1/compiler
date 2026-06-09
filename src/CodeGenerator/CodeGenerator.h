#pragma once
#include "../Common/DataType.h"
#include "../Common/Node.h"
#include "../VM/Type/Chunk.h"

#include <map>
#include <unordered_set>

class CodeGenerator final : public AstVisitor
{
public:
	std::unique_ptr<Chunk> Generate(const std::shared_ptr<AstNode>& root);

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

	void Visit(ImplStateNode& node) override;

	void Visit(TsopInitNode& node) override;

	void Visit(MethodCallNode& node) override;

	void Visit(EnumDeclNode& node) override;

	void Visit(EnumCaseNode& node) override;

	void Visit(MatchNode& node) override;

	void Visit(MatchCaseNode& node) override;

	void Visit(BreakNode& node) override;

	void Visit(ContinueNode& node) override;

private:
	void PushCode(uint8_t code);

	void PushConst(const Value& value);

	void ResolveJmp();

	void EnterScope();

	void ExitScope();

	uint8_t AddConstant(const Value& value);

	void PushTempAddr();

private:
	struct LocalVarInfo
	{
		DataType type;
		size_t index;
		bool isConst;
	};

	struct LoopContext
	{
		std::string continueTargetLabel;
		std::string breakTargetLabel;
	};
	std::vector<LoopContext> m_loopStack;

	Chunk m_chunk{};
	std::unordered_map<std::string, size_t> m_locals;
	std::vector<std::map<std::string, LocalVarInfo>> m_scopes;
	std::unordered_set<std::string> m_userFunctions;
	std::unordered_map<std::string, std::vector<std::string>> m_structs;
	size_t m_nextLocalIdx = 0;

	std::string m_currentImplSchema;
	std::string m_currentImplState;
};
