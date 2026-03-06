#include "VirtualMachine.h"

#include "OpCode.h"

#include <iostream>

void VirtualMachine::Interpret(std::unique_ptr<Chunk> chunk)
{
	m_currentChunk = std::move(chunk);
	m_iPointer = 0;
	Run();
}

void VirtualMachine::Run()
{
	while (m_iPointer < m_currentChunk->code.size())
	{
		const auto opCode = static_cast<OpCode>(ReadByte());
		switch (opCode)
		{
		case OpCode::OP_CONSTANT: {
			OpConstant();
			break;
		}
		case OpCode::OP_RETURN:
			if (m_stack.empty())
			{
				return;
			}
			OpReturn();
			return;
		case OpCode::OP_ADD:
			OpAdd();
			break;
		case OpCode::OP_SUB:
			OpSub();
			break;
		case OpCode::OP_NEG:
			OpNeg();
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
	std::cout << "\nOn return: ";
	PrintValue(m_stack.top());
}

void VirtualMachine::OpAdd()
{
	const auto res = Pop() + Pop();
	Push(res);
}

void VirtualMachine::OpSub()
{
	const auto res = Pop() - Pop();
	Push(res);
}

void VirtualMachine::OpNeg()
{
	Push(-Pop());
}

void VirtualMachine::Push(const Value& value)
{
	m_stack.push(value);
}

Value VirtualMachine::Pop()
{
	if (m_stack.empty())
	{
		return {};
	}
	const auto top = m_stack.top();
	m_stack.pop();
	return top;
}

uint8_t VirtualMachine::ReadByte()
{
	return m_currentChunk->code[m_iPointer++];
}