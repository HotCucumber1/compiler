#include "CodeGenerator.h"
#include "OpCode.h"

#include <ranges>

bool IsEmptyExpression(const std::shared_ptr<AstNode>& node);
std::string GenerateLabel(const std::string& prefix);

std::string CodeGenerator::GetFullCode() const
{
	std::stringstream full;
	full << "section .data\n";
	full << "    msg_true db '1', 10\n";
	full << "    msg_false db '0', 10\n";
	full << "section .text\n";
	full << "global _start\n\n";
	full << "_start:\n";

	full << m_asm.str();

	full << "    cmp rax, 0\n";
	full << "    jne .print_1\n";
	full << "    jmp .print_0\n\n";

	full << ".print_1:\n";
	full << "    mov rax, 1      ; sys_write\n";
	full << "    mov rdi, 1      ; stdout\n";
	full << "    mov rsi, msg_true\n";
	full << "    mov rdx, 2\n";
	full << "    syscall\n";
	full << "    jmp .exit\n\n";

	full << ".print_0:\n";
	full << "    mov rax, 1      ; sys_write\n";
	full << "    mov rdi, 1      ; stdout\n";
	full << "    mov rsi, msg_false\n";
	full << "    mov rdx, 2\n";
	full << "    syscall\n\n";

	full << ".exit:\n";
	full << "    mov rax, 60     ; sys_exit\n";
	full << "    xor rdi, rdi    ; code 0\n";
	full << "    syscall\n";

	return full.str();
}

std::unique_ptr<Chunk> CodeGenerator::Generate(const std::shared_ptr<AstNode>& root)
{
	m_chunk = Chunk();
	m_nextLocalIdx = 0;
	m_scopes.clear();

	EnterScope();
	if (root)
	{
		root->Accept(*this);
	}
	ExitScope();

	PushCode(static_cast<uint8_t>(OpCode::OP_RETURN));
	ResolveJmp();
	return std::make_unique<Chunk>(m_chunk);
}

void CodeGenerator::Visit(TerminalNode& node)
{
	// Terminal does not generate some code. It is just a value
}

void CodeGenerator::Visit(CommonNode& node)
{
	if (node.type == AstNodeType::UNDEFINED
		&& node.children.size() == 1)
	{
		if (node.children[0])
		{
			node.children[0]->Accept(*this);
		}
		return;
	}
	for (auto& child : node.children)
	{
		if (child)
		{
			child->Accept(*this);
		}
	}
}

void CodeGenerator::Visit(BinaryOpNode& node)
{
	if (node.left)
	{
		node.left->Accept(*this);
	}
	if (node.right)
	{
		node.right->Accept(*this);
	}
	const auto leftType = node.left->type; // TODO где-то надо сетать
	const auto rightType = node.left->type;
	m_asm << "; binary operation node \n";

	m_asm << "    pop rbx         ; rhs\n";
	m_asm << "    pop rax         ; lhs\n";

	switch (node.op.type)
	{
	case TokenType::OP_PLUS:
		m_asm << "    add rax, rbx\n";
		PushCode(static_cast<uint8_t>(OpCode::OP_ADD));
		break;
	case TokenType::OP_MINUS:
		m_asm << "    sub rax, rbx\n";
		PushCode(static_cast<uint8_t>(OpCode::OP_SUB));
		break;
	case TokenType::OP_MULTI:
		m_asm << "    imul rax, rbx\n";
		PushCode(static_cast<uint8_t>(OpCode::OP_MULTI));
		break;
	case TokenType::OP_DIV:
		m_asm << "    cqo\n    idiv rbx\n";
		PushCode(static_cast<uint8_t>(OpCode::OP_DIV));
		break;
	case TokenType::OP_MOD:
		PushCode(static_cast<uint8_t>(OpCode::OP_MOD));
		break;
	case TokenType::OP_EQUAL:
		m_asm << "    cmp rax, rbx\n";
		m_asm << "    sete al\n";
		m_asm << "    movzx rax, al\n";
		PushCode(static_cast<uint8_t>(OpCode::OP_EQUAL));
		break;
	case TokenType::OP_NOT_EQUAL:
		PushCode(static_cast<uint8_t>(OpCode::OP_NOT_EQUAL));
		break;
	case TokenType::OP_LESS:
		m_asm << "    cmp rax, rbx\n";
		m_asm << "    setl al\n";
		m_asm << "    movzx rax, al\n";
		PushCode(static_cast<uint8_t>(OpCode::OP_LESS));
		break;
	case TokenType::OP_MORE:
		m_asm << "    cmp rax, rbx\n";
		m_asm << "    setg al\n";
		m_asm << "    movzx rax, al\n";
		PushCode(static_cast<uint8_t>(OpCode::OP_GREATER));
		break;
	case TokenType::AND:
		m_asm << "; AND\n";
		m_asm << "    test rax, rax\n";
		m_asm << "    setne al\n";
		m_asm << "    test rbx, rbx\n";
		m_asm << "    setne bl\n";
		m_asm << "    and al, bl\n";
		m_asm << "    movzx rax, al\n";
		PushCode(static_cast<uint8_t>(OpCode::OP_AND));
		break;
	case TokenType::OR:
		m_asm << "; OR\n";
		m_asm << "    test rax, rax\n";
		m_asm << "    setne al\n";
		m_asm << "    test rbx, rbx\n";
		m_asm << "    setne bl\n";
		m_asm << "    or al, bl\n";
		m_asm << "    movzx rax, al\n";
		PushCode(static_cast<uint8_t>(OpCode::OP_OR));
		break;
	default:
		break;
	}
	m_asm << "    push rax\n";
}

void CodeGenerator::Visit(UnaryOpNode& node)
{
	if (node.operand)
	{
		node.operand->Accept(*this);
	}
	m_asm << "; unary operation node \n";
	m_asm << "    pop rax\n";
	switch (node.op.type)
	{
	case TokenType::OP_NEG:
	case TokenType::OP_MINUS:
		m_asm << "    neg rax\n";
		PushCode(static_cast<uint8_t>(OpCode::OP_NEG));
		break;
	case TokenType::NOT:
		m_asm << "    cmp rax, 0\n    setz al\n    movzx rax, al\n";
		PushCode(static_cast<uint8_t>(OpCode::OP_NOT));
		break;
	default:
		break;
	}
}

void CodeGenerator::Visit(VarDeclNode& node)
{
	if (m_scopes.back().contains(node.varName))
	{
		throw std::runtime_error("Variable already declared: " + node.varName);
	}

	const auto idx = m_nextLocalIdx++;
	m_scopes.back()[node.varName] = { DataType::UNKNOWN, idx, node.isConst };
	m_chunk.localsCount = std::max(m_chunk.localsCount, m_nextLocalIdx);

	if (node.expr)
	{
		node.expr->Accept(*this);
		PushCode(static_cast<uint8_t>(OpCode::OP_SET_LOCAL_VAR));
		PushCode(static_cast<uint8_t>(idx));
	}
}

void CodeGenerator::Visit(LiteralNode& node)
{
	Value val;
	m_asm << "; literal node \n";
	if (node.token.type == TokenType::INT)
	{
		val = Value(std::stoi(node.token.value));
		m_asm << "    push " << node.token.value << std::endl;
	}
	else if (node.token.type == TokenType::FLOAT)
	{
		val = Value(std::stof(node.token.value));
	}
	else if (node.token.type == TokenType::TRUE)
	{
		val = Value(true);
		m_asm << "    push 1 ; true\n";
	}
	else if (node.token.type == TokenType::FALSE)
	{
		val = Value(false);
		m_asm << "    push 0 ; false\n";
	}
	else if (node.token.type == TokenType::STRING)
	{
		const auto rawStr = node.token.value;
		const auto cleanStr = rawStr.substr(1, rawStr.size() - 2);
		val = MakeString(cleanStr);
	}
	PushConst(val);
}

void CodeGenerator::Visit(VarAccessNode& node)
{
	for (auto& m_scope : std::ranges::reverse_view(m_scopes))
	{
		if (!m_scope.contains(node.varName))
		{
			continue;
		}

		const auto idx = m_scope[node.varName].index;
		PushCode(static_cast<uint8_t>(OpCode::OP_GET_LOCAL_VAR));
		PushCode(idx);
		return;
	}
	throw std::runtime_error("Error: undefined variable: " + node.varName);
}

void CodeGenerator::Visit(AssignmentNode& node)
{
	size_t varIdx = -1;
	for (auto& m_scope : std::ranges::reverse_view(m_scopes))
	{
		if (m_scope.contains(node.varName))
		{
			varIdx = m_scope[node.varName].index;
			break;
		}
	}

	if (varIdx == -1)
	{
		throw std::runtime_error("Assignment error: Undefined variable '" + node.varName + "'");
	}
	if (node.expr)
	{
		node.expr->Accept(*this);
	}
	PushCode(static_cast<uint8_t>(OpCode::OP_SET_LOCAL_VAR));
	PushCode(static_cast<uint8_t>(varIdx));
}

void CodeGenerator::Visit(IfNode& node)
{
	m_asm << "; if node \n";
	const auto elseAsmLabel = NextLabel("else");
	const auto endAsmLabel = NextLabel("endif");

	if (node.condition)
	{
		node.condition->Accept(*this);
		m_asm << "    pop rax\n";
		m_asm << "    cmp rax, 0\n";
		m_asm << "    je " << elseAsmLabel << std::endl;
	}

	PushCode(static_cast<uint8_t>(OpCode::OP_JMP_IF_FALSE));
	const auto elseLabel = GenerateLabel("else_or_end");
	const auto jumpToElseIdx = m_chunk.code.size();
	PushTempAddr();

	if (node.thenBranch)
	{
		node.thenBranch->Accept(*this);
	}
	m_asm << "    jmp " << endAsmLabel << "\n";
	m_asm << elseAsmLabel << ":\n";
	if (node.elseBranch)
	{
		PushCode(static_cast<uint8_t>(OpCode::OP_JMP));
		const auto endLabel = GenerateLabel("if_end");
		const auto jumpToEndIdx = m_chunk.code.size();
		PushTempAddr();

		m_chunk.labels[elseLabel] = m_chunk.code.size();
		node.elseBranch->Accept(*this);

		m_chunk.labels[endLabel] = m_chunk.code.size();
		m_chunk.pendingJumps.emplace_back(jumpToEndIdx, endLabel);
	}
	else
	{
		m_chunk.labels[elseLabel] = m_chunk.code.size();
	}
	m_asm << endAsmLabel << ":\n";
	m_chunk.pendingJumps.emplace_back(jumpToElseIdx, elseLabel);
}

void CodeGenerator::Visit(WhileNode& node)
{
	const auto startLabel = GenerateLabel("while_start");
	const auto endLabel = GenerateLabel("while_end");

	m_chunk.labels[startLabel] = m_chunk.code.size();

	if (node.condition)
	{
		node.condition->Accept(*this);
	}

	PushCode(static_cast<uint8_t>(OpCode::OP_JMP_IF_FALSE));
	auto jumpToEndIdx = m_chunk.code.size();
	PushTempAddr();
	m_chunk.pendingJumps.emplace_back(jumpToEndIdx, endLabel);

	if (node.body)
	{
		node.body->Accept(*this);
	}

	PushCode(static_cast<uint8_t>(OpCode::OP_JMP));
	auto jumpToStartIdx = m_chunk.code.size();
	PushTempAddr();
	m_chunk.pendingJumps.emplace_back(jumpToStartIdx, startLabel);

	m_chunk.labels[endLabel] = m_chunk.code.size();
}

void CodeGenerator::Visit(ForNode& node)
{
	if (node.init)
	{
		node.init->Accept(*this);
	}

	const auto startLabel = GenerateLabel("for_while_start");
	const auto endLabel = GenerateLabel("for_while_end");
	m_chunk.labels[startLabel] = m_chunk.code.size();

	if (node.condition && !IsEmptyExpression(node.condition))
	{
		node.condition->Accept(*this);
	}
	else
	{
		PushConst(Value(true));
	}

	PushCode(static_cast<uint8_t>(OpCode::OP_JMP_IF_FALSE));
	auto jumpToEndIdx = m_chunk.code.size();
	PushTempAddr();
	m_chunk.pendingJumps.emplace_back(jumpToEndIdx, endLabel);

	if (node.body)
	{
		node.body->Accept(*this);
	}

	if (node.increment && !IsEmptyExpression(node.increment))
	{
		node.increment->Accept(*this);
	}

	PushCode(static_cast<uint8_t>(OpCode::OP_JMP));
	auto jumpToStartIdx = m_chunk.code.size();
	PushTempAddr();
	m_chunk.pendingJumps.emplace_back(jumpToStartIdx, startLabel);

	m_chunk.labels[endLabel] = m_chunk.code.size();
}

void CodeGenerator::Visit(CallNode& node)
{
	const auto nameIdx = AddConstant(MakeString(node.funcName));
	PushCode(static_cast<uint8_t>(OpCode::OP_GET_GLOBAL));
	PushCode(nameIdx);

	for (const auto& arg : node.args)
	{
		if (arg)
		{
			arg->Accept(*this);
		}
	}
	PushCode(static_cast<uint8_t>(OpCode::OP_CALL));
	PushCode(node.args.size());
}

void CodeGenerator::Visit(FunctionDeclNode& node)
{
	auto globalChunk = std::move(m_chunk);
	auto globalScopes = std::move(m_scopes);
	const auto globalNextLocal = m_nextLocalIdx;

	m_chunk = Chunk();
	m_scopes.clear();
	m_nextLocalIdx = 0;

	EnterScope();
	for (const auto& param : node.parameters)
	{
		m_scopes.back()[param] = { DataType::UNKNOWN, m_nextLocalIdx++, false };
	}
	m_chunk.localsCount = m_nextLocalIdx;

	if (node.body)
	{
		node.body->Accept(*this);
	}

	PushConst(Value{});
	PushCode(static_cast<uint8_t>(OpCode::OP_RETURN));
	ResolveJmp();

	auto funcName = String(node.name);
	auto functionObj = std::make_shared<Function>(&funcName, node.parameters.size());
	functionObj->chunk = std::move(m_chunk);

	m_chunk = std::move(globalChunk);
	m_scopes = std::move(globalScopes);
	m_nextLocalIdx = globalNextLocal;

	const auto funcIdx = AddConstant(Value(functionObj));
	const auto nameIdx = AddConstant(MakeString(node.name));

	PushCode(static_cast<uint8_t>(OpCode::OP_CONSTANT));
	PushCode(funcIdx);
	PushCode(static_cast<uint8_t>(OpCode::OP_SET_GLOBAL));
	PushCode(nameIdx);
}

void CodeGenerator::Visit(StructDeclNode& node)
{
	std::vector<std::string> fieldNames;
	for (const auto& fieldNode : node.fields)
	{
		if (const auto varDecl = std::dynamic_pointer_cast<VarDeclNode>(fieldNode))
		{
			fieldNames.push_back(varDecl->varName);
		}
	}
	m_structs[node.name] = std::move(fieldNames);
}

void CodeGenerator::Visit(StructInitNode& node)
{
	if (!m_structs.contains(node.structName))
	{
		throw std::runtime_error("Compile error: Unknown struct '" + node.structName + "'");
	}
	const auto& fieldNames = m_structs[node.structName];
	if (node.args.size() > fieldNames.size())
	{
		throw std::runtime_error("Compile error: Too many arguments for struct '" + node.structName + "'");
	}

	const auto nameIdx = AddConstant(MakeString(node.structName));
	PushCode(static_cast<uint8_t>(OpCode::OP_STRUCT_INIT));
	PushCode(nameIdx);

	for (int i = 0; i < node.args.size(); i++)
	{
		PushCode(static_cast<uint8_t>(OpCode::OP_DUP));
		if (node.args[i])
		{
			node.args[i]->Accept(*this);
		}

		const auto fieldNameIdx = AddConstant(MakeString(fieldNames[i]));
		PushCode(static_cast<uint8_t>(OpCode::OP_SET_FIELD));
		PushCode(fieldNameIdx);

		PushCode(static_cast<uint8_t>(OpCode::OP_POP));
	}
}

void CodeGenerator::Visit(MemberAccessNode& node)
{
	node.object->Accept(*this);

	const auto nameIdx = AddConstant(MakeString(node.fieldName));
	PushCode(static_cast<uint8_t>(OpCode::OP_GET_FIELD));
	PushCode(nameIdx);
}

void CodeGenerator::Visit(MemberAssignmentNode& node)
{
	if (node.object)
	{
		node.object->Accept(*this);
	}
	if (node.expression)
	{
		node.expression->Accept(*this);
	}
	const auto fieldNameIdx = AddConstant(MakeString(node.fieldName));
	PushCode(static_cast<uint8_t>(OpCode::OP_SET_FIELD));
	PushCode(fieldNameIdx);
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

void CodeGenerator::PushTempAddr()
{
	PushCode(0xFF);
	PushCode(0xFF);
}

std::string GenerateLabel(const std::string& prefix)
{
	static int counter = 0;
	return prefix + "_" + std::to_string(counter++);
}

bool IsEmptyExpression(const std::shared_ptr<AstNode>& node)
{
	if (!node)
	{
		return true;
	}
	if (const auto common = std::dynamic_pointer_cast<CommonNode>(node))
	{
		return common->type == AstNodeType::UNDEFINED
			&& common->children.empty();
	}
	return false;
}
