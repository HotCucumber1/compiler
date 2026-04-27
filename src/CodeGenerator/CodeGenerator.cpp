#include "CodeGenerator.h"

#include <ranges>

#include "OpCode.h"

bool IsBinaryOp(const std::shared_ptr<AstNode>& node);
bool IsLiteral(const std::shared_ptr<AstNode>& node);
bool IsVariableDecl(const std::shared_ptr<AstNode>& node);
bool IsIfStatement(const std::shared_ptr<AstNode>& node);
std::string GenerateLabel(const std::string& prefix);

std::unique_ptr<Chunk> CodeGenerator::Generate(const std::shared_ptr<AstNode>& root)
{
	m_chunk = Chunk();
	m_nextLocalIdx = 0;
	m_scopes.clear();

	EnterScope();
	Visit(root);
	ExitScope();

	PushCode(static_cast<uint8_t>(OpCode::OP_RETURN));
	ResolveJmp();
	return std::make_unique<Chunk>(m_chunk);
}

void CodeGenerator::Visit(const std::shared_ptr<AstNode>& node)
{
	if (!node)
	{
		return;
	}
	switch (node->type)
	{
	case AstNodeType::VAR_DECL:
		PushVarDecl(node);
		return;
	case AstNodeType::VAR_ACCESS:
		PushVarAccess(node);
		return;
	case AstNodeType::ASSIGNMENT:
		PushAssignment(node);
		return;
	case AstNodeType::LITERAL:
		PushLiteral(node->children[0]);
		return;
	case AstNodeType::IF_STATEMENT:
		PushIf(node);
		return;
	case AstNodeType::BINARY_OP:
		PushBinaryOp(node);
		return;
	case AstNodeType::TERMINAL:
		return;
	case AstNodeType::CALL:
		PushCall(node);
		return;

	default:
		for (auto& child : node->children)
		{
			Visit(child);
		}
		break;
	}
}

void CodeGenerator::PushCode(const uint8_t code)
{
	m_chunk.code.push_back(code);
}

void CodeGenerator::PushConst(const Value& value)
{
	const auto index = AddConstant(value);
	PushCode(static_cast<uint8_t>(OpCode::OP_CONSTANT));
	PushCode(index);
}

void CodeGenerator::PushBinaryOp(const std::shared_ptr<AstNode>& node)
{
	Visit(node->children[0]);
	Visit(node->children[2]);

	switch (node->children[1]->token->type)
	{
	case TokenType::OP_PLUS:
		PushCode(static_cast<uint8_t>(OpCode::OP_ADD));
		break;
	case TokenType::OP_MINUS:
		PushCode(static_cast<uint8_t>(OpCode::OP_SUB));
		break;
	case TokenType::OP_MULTI:
		PushCode(static_cast<uint8_t>(OpCode::OP_MULTI));
		break;
	case TokenType::OP_DIV:
		PushCode(static_cast<uint8_t>(OpCode::OP_DIV));
		break;
	case TokenType::OP_MOD:
		PushCode(static_cast<uint8_t>(OpCode::OP_MOD));
		break;
	case TokenType::OP_EQUAL:
		PushCode(static_cast<uint8_t>(OpCode::OP_EQUAL));
		break;
	case TokenType::OP_NOT_EQUAL:
		PushCode(static_cast<uint8_t>(OpCode::OP_NOT_EQUAL));
		break;
	case TokenType::OP_LESS:
		PushCode(static_cast<uint8_t>(OpCode::OP_LESS));
		break;
	case TokenType::OP_MORE:
		PushCode(static_cast<uint8_t>(OpCode::OP_GREATER));
		break;
	default:
		break;
	}
}

void CodeGenerator::PushVarDecl(const std::shared_ptr<AstNode>& node)
{
	constexpr int exprPos = 3;
	if (node->children.size() < exprPos - 1 || !node->children[1]->token)
	{
		return;
	}

	const auto varName = node->children[1]->token->value;

	if (m_scopes.back().contains(varName))
	{
		throw std::runtime_error("Variable already declared: " + varName);
	}

	const auto idx = m_nextLocalIdx++;
	m_scopes.back()[varName] = idx;
	m_chunk.localsCount = std::max(m_chunk.localsCount, m_nextLocalIdx);

	if (node->children.size() > exprPos)
	{
		Visit(node->children[exprPos]);

		PushCode(static_cast<uint8_t>(OpCode::OP_SET_LOCAL_VAR));
		PushCode(static_cast<uint8_t>(idx));
	}
}

void CodeGenerator::PushLiteral(const std::shared_ptr<AstNode>& node)
{
	Value val;
	if (node->token->type == TokenType::INT)
		val = Value(std::stoi(node->token->value));
	else if (node->token->type == TokenType::FLOAT)
		val = Value(std::stof(node->token->value));
	else if (node->token->type == TokenType::TRUE)
		val = Value(true);
	else if (node->token->type == TokenType::FALSE)
		val = Value(false);
	else if (node->token->type == TokenType::STRING)
		val = MakeString(node->token->value);

	PushConst(val);
}

void CodeGenerator::PushIf(const std::shared_ptr<AstNode>& node)
{
	constexpr int elseSize = 7;
	constexpr int elseIfSize = 9;
	Visit(node->children[2]);

	PushCode(static_cast<uint8_t>(OpCode::OP_JMP_IF_FALSE));
	const auto elseLabel = GenerateLabel("else_or_end");
	const auto jumpToElseIdx = m_chunk.code.size();
	PushCode(0xFF);
	PushCode(0xFF);

	Visit(node->children[5]);

	if (node->children.size() > elseSize)
	{
		PushCode(static_cast<uint8_t>(OpCode::OP_JMP));
		std::string endLabel = GenerateLabel("if_end");
		size_t jumpToEndIdx = m_chunk.code.size();
		PushCode(0xFF);
		PushCode(0xFF);

		m_chunk.labels[elseLabel] = m_chunk.code.size();

		if (node->children.size() == elseIfSize)
		{
			Visit(node->children[elseIfSize - 1]);
		}

		else if (node->children.size() == elseSize + 4)
		{
			Visit(node->children[9]);
		}

		m_chunk.labels[endLabel] = m_chunk.code.size();
		m_chunk.pendingJumps.emplace_back(jumpToEndIdx, endLabel);
	}
	else
	{
		m_chunk.labels[elseLabel] = m_chunk.code.size();
	}
	m_chunk.pendingJumps.emplace_back(jumpToElseIdx, elseLabel);
}

void CodeGenerator::PushCall(const std::shared_ptr<AstNode>& node)
{
	if (node->children.size() < 3)
	{
		return;
	}

	const auto funcName = node->children[0]->token->value;

	const uint8_t nameIdx = AddConstant(MakeString(funcName));
	PushCode(static_cast<uint8_t>(OpCode::OP_GET_GLOBAL));
	PushCode(nameIdx);

	Visit(node->children[2]);

	PushCode(static_cast<uint8_t>(OpCode::OP_CALL));
	PushCode(1); // TODO кол во аргументов
}

void CodeGenerator::PushVarAccess(const std::shared_ptr<AstNode>& node)
{
	const auto varName = node->children[0]->token->value;

	for (auto& m_scope : std::ranges::reverse_view(m_scopes))
	{
		if (!m_scope.contains(varName))
		{
			continue;
		}
		const auto idx = m_scope[varName];
		PushCode(static_cast<uint8_t>(OpCode::OP_GET_LOCAL_VAR));
		PushCode(idx);
		return;
	}

	throw std::runtime_error("Undefined variable: " + varName);
}

void CodeGenerator::PushAssignment(const std::shared_ptr<AstNode>& node)
{
	const auto varName = node->children[0]->token->value;

	int varIdx = -1;
	for (auto& m_scope : std::ranges::reverse_view(m_scopes))
	{
		if (m_scope.contains(varName))
		{
			varIdx = m_scope[varName];
			break;
		}
	}

	if (varIdx == -1)
	{
		throw std::runtime_error("Assignment error: Undefined variable '" + varName + "'");
	}

	Visit(node->children[2]);
	PushCode(static_cast<uint8_t>(OpCode::OP_SET_LOCAL_VAR));
	PushCode(static_cast<uint8_t>(varIdx));
}

void CodeGenerator::ResolveJmp()
{
	for (auto& [pos, labelName] : m_chunk.pendingJumps)
	{
		constexpr int byteSize = 8;
		if (!m_chunk.labels.contains(labelName))
		{
			throw std::runtime_error("Undefined label: " + labelName);
		}
		const auto targetAddress = m_chunk.labels[labelName];
		m_chunk.code[pos] = (targetAddress >> byteSize) & 0xFF;
		m_chunk.code[pos + 1] = targetAddress & 0xFF;
	}
}

void CodeGenerator::EnterScope()
{
	m_scopes.emplace_back();
}

void CodeGenerator::ExitScope()
{
	m_scopes.pop_back();
}
uint8_t CodeGenerator::AddConstant(const Value& value)
{
	m_chunk.constants.push_back(value);
	return static_cast<uint8_t>(m_chunk.constants.size() - 1);
}

bool IsBinaryOp(const std::shared_ptr<AstNode>& node)
{
	if (!node->token)
	{
		return false;
	}
	const auto t = node->token->type;
	return t == TokenType::OP_PLUS
		|| t == TokenType::OP_MINUS
		|| t == TokenType::OP_MULTI
		|| t == TokenType::OP_DIV
		|| t == TokenType::OP_MOD;
}

bool IsLiteral(const std::shared_ptr<AstNode>& node)
{
	if (!node->token)
	{
		return false;
	}
	const auto t = node->token->type;
	return t == TokenType::INT
		|| t == TokenType::FLOAT
		|| t == TokenType::STRING
		|| t == TokenType::TRUE
		|| t == TokenType::FALSE;
}

bool IsVariableDecl(const std::shared_ptr<AstNode>& node)
{
	return node->token
		&& (node->token->type == TokenType::VAR || node->token->type == TokenType::CONST);
}

bool IsIfStatement(const std::shared_ptr<AstNode>& node)
{
	return node->token && node->token->type == TokenType::IF;
}

std::string GenerateLabel(const std::string& prefix)
{
	static int counter = 0;
	return prefix + "_" + std::to_string(counter++);
}
