#pragma once
#include "../Frontend/Parser/Common/Node.h"
#include "../VM/Type/Chunk.h"

#include <map>

class CodeGenerator : public AstVisitor
{
public:
	// TODO в m_scopes добавить типы, чтобы проверять еще до VM (проверка семантики)
	// TODO приведение типов тоже на семантике (float -
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
		size_t index;
		bool isConst;
	};

	Chunk m_chunk{};
	std::unordered_map<std::string, size_t> m_locals;
	std::vector<std::map<std::string, LocalVarInfo>> m_scopes;
	size_t m_nextLocalIdx = 0;
};
