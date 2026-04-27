#pragma once
#include "../Frontend/Parser/Common/Node.h"
#include "../VM/Type/Chunk.h"

#include <map>

class CodeGenerator
{
public:
	std::unique_ptr<Chunk> Generate(const std::shared_ptr<AstNode>& root);

private:
	void Visit(const std::shared_ptr<AstNode>& node);

	void PushCode(uint8_t code);

	void PushConst(const Value& value);

	void PushBinaryOp(const std::shared_ptr<AstNode>& node);

	void PushVarDecl(const std::shared_ptr<AstNode>& node);

	void PushLiteral(const std::shared_ptr<AstNode>& node);

	void PushIf(const std::shared_ptr<AstNode>& node);

	void PushCall(const std::shared_ptr<AstNode>& node);

	void PushVarAccess(const std::shared_ptr<AstNode>& node);

	void PushAssignment(const std::shared_ptr<AstNode>& node);

	void ResolveJmp();

	void EnterScope();

	void ExitScope();

	uint8_t AddConstant(const Value& value);

private:
	Chunk m_chunk{};
	std::unordered_map<std::string, size_t> m_locals;
	std::vector<std::map<std::string, size_t>> m_scopes;
	size_t m_nextLocalIdx = 0;
};
