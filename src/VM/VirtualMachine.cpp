#include "VirtualMachine.h"
#include "OpCode.h"

#include <algorithm>
#include <iostream>
#include <pstl/glue_execution_defs.h>

constexpr int BYTE_SIZE = 8;


void VirtualMachine::Interpret(std::unique_ptr<Chunk> chunk)
{
	const auto mainFunc = std::make_shared<Function>(nullptr, 0);
	mainFunc->chunk = std::move(*chunk);

	const CallFrame mainFrame(mainFunc);
	m_callFrames.push_back(mainFrame);

	m_valueStack.clear();
	Run();
}

Value VirtualMachine::GetStackTop() const
{
	if (m_valueStack.empty())
	{
		throw std::runtime_error("Stack is empty");
	}
	return m_valueStack.back();
}

Value VirtualMachine::GetLocal(const size_t index) const
{
	if (m_callFrames.empty())
	{
		throw std::runtime_error("No call frame");
	}
	const auto& locals = m_callFrames.back().locals;
	if (index >= locals.size())
	{
		throw std::runtime_error("Local variable index out of range");
	}
	return locals[index];
}

void VirtualMachine::Run()
{
	while (true)
	{
		if (m_callFrames.empty())
		{
			return;
		}
		CallFrame& currentFrame = m_callFrames.back();
		m_currentChunk = &currentFrame.function->chunk;
		m_instructionPointer = currentFrame.instructionPointer;

		if (m_instructionPointer >= m_currentChunk->code.size())
		{
			OpReturn();
			continue;
		}
		switch (const auto opCode = static_cast<OpCode>(ReadByte()))
		{
		case OpCode::OP_CONSTANT:
			OpConstant();
			break;
		case OpCode::OP_RETURN:
			OpReturn();
			continue;
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
		case OpCode::OP_JMP:
			OpJmp();
			break;
		case OpCode::OP_JMP_IF_FALSE:
			OpJmpIfFalse();
			break;
		case OpCode::OP_CALL:
			OpCall();
			break;
		case OpCode::OP_FUNC:
			OpFunc();
			break;
		default:
			std::cerr << "Unknown operation: " << static_cast<int>(opCode) << std::endl;
			return;
		}
		currentFrame.instructionPointer = m_instructionPointer;
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
	Value returnValue;
	if (!m_valueStack.empty())
	{
		returnValue = Pop();
	}

	m_callFrames.pop_back();
	Push(returnValue);
	if (!m_callFrames.empty())
	{
		std::cout << "Returned: ";
		PrintValue(returnValue);
		std::cout << std::endl;
	}
	else
	{
		std::cout << "\nProgram finished. Result: ";
		PrintValue(returnValue);
		std::cout << std::endl;
	}
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
	const auto value = Pop();

	if (m_callFrames.empty())
	{
		throw std::runtime_error("No call frame");
	}

	auto& locals = m_callFrames.back().locals;
	if (index >= locals.size())
	{
		throw std::runtime_error("Local variable index out of range");
	}
	locals[index] = value;
}

void VirtualMachine::OpGetLocal()
{
	const auto index = ReadByte();
	if (m_callFrames.empty())
	{
		throw std::runtime_error("No call frame");
	}

	const auto& locals = m_callFrames.back().locals;
	if (index >= locals.size())
	{
		throw std::runtime_error("Local variable index out of range");
	}
	Push(locals[index]);
}

void VirtualMachine::OpJmp()
{
	const auto address = static_cast<uint16_t>(ReadByte()) << BYTE_SIZE | ReadByte();
	m_instructionPointer = address;
}

void VirtualMachine::OpJmpIfFalse()
{
	const auto address = static_cast<uint16_t>(ReadByte()) << BYTE_SIZE | ReadByte();
	const auto condition = Pop();

	if (!condition)
	{
		m_instructionPointer = address;
	}
}

void VirtualMachine::OpCall()
{
	const auto argCount = ReadByte();

	const auto callee = Peek(argCount);
	if (!IsFunction(callee))
	{
		throw std::runtime_error("Can only call functions");
	}

	const auto function = AsFunc(callee);
	if (argCount != function->arity)
	{
		throw std::runtime_error("Expected " + std::to_string(function->arity) + " arguments but got " + std::to_string(argCount));
	}

	CallFrame frame(function);
	for (int i = argCount - 1; i >= 0; i--)
	{
		const auto arg = m_valueStack[m_valueStack.size() - 1 - i];
		frame.locals[i] = arg;
	}
	for (int i = 0; i <= argCount; i++)
	{
		m_valueStack.pop_back();
	}

	CallFrame& currentFrame = m_callFrames.back();
	currentFrame.instructionPointer = m_instructionPointer;

	m_callFrames.push_back(frame);
}

void VirtualMachine::OpFunc()
{
	const auto funcIndex = ReadByte();
	if (funcIndex >= m_currentChunk->constants.size())
	{
		throw std::runtime_error("Function index out of range");
	}

	const auto funcValue = m_currentChunk->constants[funcIndex];

	if (!IsFunction(funcValue))
	{
		throw std::runtime_error("Expected function constant");
	}

	Push(funcValue);
}

Value VirtualMachine::Peek(const int distance) const
{
	if (distance >= static_cast<int>(m_valueStack.size()))
	{
		throw std::runtime_error("Stack index out of range");
	}
	return m_valueStack[m_valueStack.size() - 1 - distance];
}

uint16_t VirtualMachine::ReadShort()
{
	return static_cast<uint16_t>(ReadByte()) << BYTE_SIZE | ReadByte();
}

void VirtualMachine::Push(const Value& value)
{
	m_valueStack.push_back(value);
}

Value VirtualMachine::Pop()
{
	if (m_valueStack.empty())
	{
		throw std::runtime_error("Const stack is empty");
	}
	const auto top = m_valueStack.back();
	m_valueStack.pop_back();
	return top;
}

uint8_t VirtualMachine::ReadByte()
{
	return m_currentChunk->code[m_instructionPointer++];
}
