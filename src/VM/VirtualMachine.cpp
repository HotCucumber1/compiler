#include "VirtualMachine.h"
#include "OpCode.h"

#include <algorithm>
#include <iostream>

constexpr int BYTE_SIZE = 8;

Value NativePrint(const std::vector<Value>& args)
{
	for (const auto& arg : args)
	{
		PrintValue(arg);
	}
	return {};
}

VirtualMachine::VirtualMachine()
{
	RegisterNativeFunction("print", 1, NativePrint);
	// TODO чтение
}

void VirtualMachine::Interpret(std::unique_ptr<Chunk> chunk)
{
	const auto mainFunc = std::make_shared<Function>(nullptr, 0);
	mainFunc->chunk = std::move(*chunk);

	const CallFrame mainFrame(mainFunc);
	m_callFrames.push_back(mainFrame);

	m_valueStack.clear();
	Run();
}

void VirtualMachine::RegisterNativeFunction(const std::string& name, int arity, const NativeFunction::NativeFunc& func)
{
	auto native = std::make_shared<NativeFunction>(name, arity, func);
	m_globals[name] = native;
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
		case OpCode::OP_JMP_IF_LESS:
			OpJmpIfLess();
			break;
		case OpCode::OP_JMP_IF_GREATER:
			OpJmpIfGreater();
			break;
		case OpCode::OP_CALL:
			OpCall();
			break;
		case OpCode::OP_FUNC:
			OpFunc();
			break;
		case OpCode::OP_CREATE_ARRAY:
			OpCreateArr();
			break;
		case OpCode::OP_GET_ELEMENT:
			OpGetEl();
			break;
		case OpCode::OP_SET_ELEMENT:
			OpSetEl();
			break;
		case OpCode::OP_GET_GLOBAL:
			OpGetGlobal();
			break;
		case OpCode::OP_SET_GLOBAL:
			OpSetGlobal();
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
	}
	else
	{
		std::cout << "\nProgram finished.\nResult: \n";
		PrintValue(returnValue);
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
void VirtualMachine::OpJmpIfLess()
{
	const auto address = static_cast<uint16_t>(ReadByte()) << BYTE_SIZE | ReadByte();
	const auto value = Pop();

	if (!std::holds_alternative<int64_t>(value) && !std::holds_alternative<double>(value))
	{
		throw std::runtime_error("Condition for JMP_IF_LESS must be numeric");
	}

	if (std::holds_alternative<int64_t>(value) && std::get<int64_t>(value) < 0
		|| std::holds_alternative<double>(value) && std::get<double>(value) < 0)
	{
		m_instructionPointer = address;
	}
}
void VirtualMachine::OpJmpIfGreater()
{
	const auto address = static_cast<uint16_t>(ReadByte()) << BYTE_SIZE | ReadByte();
	const auto value = Pop();

	if (!std::holds_alternative<int64_t>(value) && !std::holds_alternative<double>(value))
	{
		throw std::runtime_error("Condition for JMP_IF_GREATER must be numeric");
	}

	if (std::holds_alternative<int64_t>(value) && std::get<int64_t>(value) > 0
		|| std::holds_alternative<double>(value) && std::get<double>(value) > 0)
	{
		m_instructionPointer = address;
	}
}

void VirtualMachine::OpCall()
{
	const auto argCount = ReadByte();

	const auto callee = Peek(argCount);
	if (IsNativeFunction(callee))
	{
		auto native = AsNativeFunction(callee);
		if (argCount != native->arity)
		{
			throw std::runtime_error("Expected " + std::to_string(native->arity) + " arguments but got " + std::to_string(argCount));
		}

		std::vector<Value> args;
		for (int i = argCount - 1; i >= 0; --i)
		{
			args.push_back(m_valueStack[m_valueStack.size() - 1 - i]);
		}

		for (int i = 0; i <= argCount; ++i)
		{
			m_valueStack.pop_back();
		}
		Value result = native->func(args);
		Push(result);
		return;
	}
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

void VirtualMachine::OpCreateArr()
{
	const auto count = ReadByte();
	auto array = std::make_shared<Array>(count);

	for (int i = count - 1; i >= 0; --i)
	{
		array->elements[i] = Pop();
	}
	Push(array);
}

void VirtualMachine::OpGetEl()
{
	const auto indexVal = Pop();
	const auto arrayVal = Pop();

	if (!IsArray(arrayVal))
	{
		throw std::runtime_error("Cannot get element from non-array");
	}

	const auto arr = AsArray(arrayVal);
	if (!std::holds_alternative<int64_t>(indexVal))
	{
		throw std::runtime_error("Array index must be integer");
	}

	const auto idx = std::get<int64_t>(indexVal);
	if (idx < 0 || idx >= static_cast<int64_t>(arr->elements.size()))
	{
		Push(Value{}); // null
	}
	else
	{
		Push(arr->elements[idx]);
	}
}

void VirtualMachine::OpSetEl()
{
	Value indexVal = Pop();
	Value arrayVal = Pop();
	Value value = Pop();

	if (!IsArray(arrayVal))
	{
		throw std::runtime_error("Cannot set element on non-array");
	}

	auto arr = AsArray(arrayVal);
	if (!std::holds_alternative<int64_t>(indexVal))
	{
		throw std::runtime_error("Array index must be integer");
	}

	auto idx = std::get<int64_t>(indexVal);
	if (idx < 0 || idx >= static_cast<int64_t>(arr->elements.size()))
	{
		throw std::runtime_error("Array index out of bounds");
	}
	arr->elements[idx] = value;
}

void VirtualMachine::OpGetGlobal()
{
	const uint8_t nameIndex = ReadByte();
	const auto& name = m_currentChunk->constants[nameIndex];
	if (!IsString(name))
	{
		throw std::runtime_error("Expected string constant for global name");
	}
	const auto nameStr = AsString(name);

	const auto it = m_globals.find(nameStr);
	if (it == m_globals.end())
	{
		throw std::runtime_error("Undefined global variable: " + nameStr);
	}
	Push(it->second);
}

void VirtualMachine::OpSetGlobal()
{
	const auto nameIndex = ReadByte();
	const auto& name = m_currentChunk->constants[nameIndex];
	if (!IsString(name))
	{
		throw std::runtime_error("Expected string constant for global name");
	}
	const auto nameStr = AsString(name);

	const auto value = Pop();
	m_globals[nameStr] = value;
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
