#include "VirtualMachine.h"
#include "OpCode.h"

#include <algorithm>
#include <iostream>
#include <pstl/glue_execution_defs.h>

VirtualMachine::VirtualMachine(const size_t localsCount)
	: m_localVars(localsCount)
{
}

void VirtualMachine::Interpret(std::unique_ptr<Chunk> chunk)
{
	m_currentChunk = std::move(chunk);
	m_iPointer = 0;
	Run();
}

Value VirtualMachine::GetStackTop() const
{
	return m_constLiterals.top();
}

Value VirtualMachine::GetLocal(const size_t index) const
{
	return m_localVars[index];
}

void VirtualMachine::Run()
{
	while (m_iPointer < m_currentChunk->code.size())
	{
		switch (const auto opCode = static_cast<OpCode>(ReadByte()))
		{
		case OpCode::OP_CONSTANT:
			OpConstant();
			break;
		case OpCode::OP_RETURN:
			OpReturn();
			return;
		case OpCode::OP_ADD:
			OpAdd();
			break;
		case OpCode::OP_SUB:
			OpSub();
			break;
		case OpCode::OP_MULTI:
			OpMulti();
			break;
		case OpCode::OP_DIV:
			OpDiv();
			break;
		case OpCode::OP_MOD:
			OpMod();
			break;
		case OpCode::OP_NEG:
			OpNeg();
			break;
		case OpCode::OP_SET_LOCAL_VAR:
			OpSetLocal();
			break;
		case OpCode::OP_GET_LOCAL_VAR:
			OpGetLocal();
			break;
		default:
			std::cerr << "Unknown operation: " << static_cast<int>(opCode) << std::endl;
			return;
		}
	}
}

void VirtualMachine::OpConstant()
{
	const auto index = ReadByte();
	const auto constant = m_currentChunk->constants[index];
	Push(constant);

	std::cout << "Pushed: ";
	PrintValue(constant);
}

void VirtualMachine::OpReturn()
{
	if (m_constLiterals.empty())
	{
		return;
	}
	std::cout << "\nOn return: ";
	PrintValue(m_constLiterals.top());
}

void VirtualMachine::OpAdd()
{
	const auto first = Pop();
	const auto second = Pop();

	Push(first + second);
}

void VirtualMachine::OpSub()
{
	const auto first = Pop();
	const auto second = Pop();

	Push(second - first);
}

void VirtualMachine::OpMulti()
{
	const auto first = Pop();
	const auto second = Pop();

	Push(first * second);
}

void VirtualMachine::OpDiv()
{
	const auto first = Pop();
	const auto second = Pop();

	Push(second / first);
}

void VirtualMachine::OpMod()
{
	const auto first = Pop();
	const auto second = Pop();

	Push(second % first);
}

void VirtualMachine::OpNeg()
{
	Push(-Pop());
}

void VirtualMachine::OpSetLocal()
{
	const auto index = ReadByte();
	m_localVars[index] = Pop();
}

void VirtualMachine::OpGetLocal()
{
	const auto index = ReadByte();
	Push(m_localVars[index]);
}

void VirtualMachine::Push(const Value& value)
{
	m_constLiterals.push(value);
}

Value VirtualMachine::Pop()
{
	if (m_constLiterals.empty())
	{
		throw std::runtime_error("Const stack is empty");
	}
	const auto top = m_constLiterals.top();
	m_constLiterals.pop();
	return top;
}

uint8_t VirtualMachine::ReadByte()
{
	return m_currentChunk->code[m_iPointer++];
}