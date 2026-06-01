#include "VirtualMachine.h"
#include "NativeLib/NativeLib.h"
#include "OpCode.h"

#include <algorithm>
#include <iostream>

constexpr int BYTE_SIZE = 8;

VirtualMachine::VirtualMachine()
{
	RegisterNativeFunction("print", 1, NativePrint);
	RegisterNativeFunction("read", 0, NativeRead);
}

void VirtualMachine::Interpret(const std::unique_ptr<Chunk>& chunk)
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
		const auto opCode = static_cast<OpCode>(ReadByte());
		switch (opCode)
		{
		case OpCode::OP_CONSTANT:
			OpConstant();
			break;
		case OpCode::OP_RETURN:
			OpReturn();
			if (m_callFrames.empty())
			{
				return;
			}
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
		case OpCode::OP_AND:
			OpAnd();
			break;
		case OpCode::OP_OR:
			OpOr();
			break;
		case OpCode::OP_NOT:
			OpNot();
			break;
		case OpCode::OP_EQUAL:
			OpEqual();
			break;
		case OpCode::OP_NOT_EQUAL:
			OpNotEqual();
			break;
		case OpCode::OP_GREATER:
			OpGrater();
			break;
		case OpCode::OP_LESS:
			OpLess();
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
			continue;
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
		case OpCode::OP_POP:
			OpPop();
			break;
		case OpCode::OP_DUP:
			OpDup();
			break;
		case OpCode::OP_STRUCT_INIT:
			OpStructInit();
			break;
		case OpCode::OP_GET_FIELD:
			OpGetField();
			break;
		case OpCode::OP_SET_FIELD:
			OpSetField();
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
}

void VirtualMachine::OpReturn()
{
	Value returnValue;
	if (!m_valueStack.empty())
	{
		returnValue = Pop();
	}

	m_callFrames.pop_back();

	if (m_callFrames.empty())
	{
		return;
	}

	auto& callerFrame = m_callFrames.back();
	m_currentChunk = &callerFrame.function->chunk;
	m_instructionPointer = callerFrame.instructionPointer;

	if (!std::holds_alternative<std::monostate>(returnValue))
	{
		Push(returnValue);
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

void VirtualMachine::OpEqual()
{
	const auto rhs = Pop();
	const auto lhs = Pop();
	Push(lhs == rhs);
}

void VirtualMachine::OpNotEqual()
{
	const auto rhs = Pop();
	const auto lhs = Pop();
	Push(lhs != rhs);
}

void VirtualMachine::OpGrater()
{
	const auto rhs = Pop();
	const auto lhs = Pop();
	Push(lhs > rhs);
}

void VirtualMachine::OpLess()
{
	const auto rhs = Pop();
	const auto lhs = Pop();
	Push(lhs < rhs);
}

void VirtualMachine::OpAnd()
{
	const auto rhs = Pop();
	const auto lhs = Pop();
	Push(lhs && rhs);
}

void VirtualMachine::OpOr()
{
	const auto rhs = Pop();
	const auto lhs = Pop();
	Push(lhs || rhs);
}

void VirtualMachine::OpNot()
{
	const auto val = Pop();
	Push(!val);
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
		for (int i = 0; i < argCount; ++i)
		{
			args.push_back(m_valueStack[m_valueStack.size() - argCount + i]);
		}

		for (int i = 0; i <= argCount; ++i)
		{
			m_valueStack.pop_back();
		}
		Value result = native->func(args);
		Push(result);

		CallFrame& currentFrame = m_callFrames.back();
		currentFrame.instructionPointer = m_instructionPointer;
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
	for (int i = 0; i < argCount; i++)
	{
		const auto arg = m_valueStack[m_valueStack.size() - argCount + i];
		frame.locals[i] = arg;
	}
	for (int i = 0; i <= argCount; i++)
	{
		m_valueStack.pop_back();
	}

	CallFrame& currentFrame = m_callFrames.back();
	currentFrame.instructionPointer = m_instructionPointer;

	m_callFrames.push_back(frame);
	m_instructionPointer = 0;
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
		throw std::runtime_error("Array index out of range");
	}
	Push(arr->elements[idx]);
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

void PrintGlobals(const std::unordered_map<std::string, Value>& m_globals)
{
	for (const auto& [key, value] : m_globals)
	{
		std::cout << key << ' ';
		PrintValue(value);
	}
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

void VirtualMachine::OpPop()
{
	Pop();
}

void VirtualMachine::OpDup()
{
	if (m_valueStack.empty())
	{
		throw std::runtime_error("Const stack is empty");
	}
	const auto top = m_valueStack.back();
	Push(top);
}

void VirtualMachine::OpStructInit()
{
	const auto nameIdx = ReadByte();
	const auto structName = AsString(m_currentChunk->constants[nameIdx]);

	auto inst = std::make_shared<Instance>(structName);
	Push(inst);
}

void VirtualMachine::OpGetField()
{
	const auto nameIdx = ReadByte();
	const auto fieldName = AsString(m_currentChunk->constants[nameIdx]);

	const auto strct = Pop();
	if (!IsInstance(strct))
	{
		throw std::runtime_error("Cannot get field from non-instance object");
	}

	const auto instance = AsInstance(strct);
	auto it = instance->fields.find(fieldName);
	if (it == instance->fields.end())
	{
		Push({});
	}
	else
	{
		Push(it->second);
	}
}

void VirtualMachine::OpSetField()
{
	const auto nameIdx = ReadByte();
	const auto fieldName = AsString(m_currentChunk->constants[nameIdx]);

	const auto val = Pop();
	const auto strct = Pop();

	if (!IsInstance(strct))
	{
		throw std::runtime_error("Cannot set field on non-instance object");
	}
	auto instance = AsInstance(strct);
	instance->fields[fieldName] = val;
	Push(val);
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
