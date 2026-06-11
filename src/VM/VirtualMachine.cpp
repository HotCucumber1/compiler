#include "VirtualMachine.h"
#include "NativeLib/NativeLib.h"
#include "NativeLib/threads/thread.h"
#include "OpCode.h"

#include <algorithm>
#include <iostream>
#include <mutex>

constexpr int BYTE_SIZE = 8;

VirtualMachine::VirtualMachine()
{
	RegisterNativeLib();
}

void VirtualMachine::Interpret(const std::unique_ptr<Chunk>& chunk)
{
	ExecutionContext mainCtx;

	const auto mainFunc = std::make_shared<Function>(nullptr, 0);
	mainFunc->chunk = std::move(*chunk);

	const CallFrame mainFrame(mainFunc);
	mainCtx.callFrames.push_back(mainFrame);
	mainCtx.currentChunk = &mainCtx.callFrames.back().function->chunk;

	Run(mainCtx);
}

void VirtualMachine::RegisterNativeFunction(const std::string& name, int arity, const NativeFunction::NativeFunc& func)
{
	std::unique_lock lock(m_globalsMutex);
	auto native = std::make_shared<NativeFunction>(name, arity, func);
	m_globals[name] = native;
}

Value VirtualMachine::GetStackTop(ExecutionContext& ctx)
{
	if (ctx.valueStack.empty())
	{
		throw std::runtime_error("Stack is empty");
	}
	return ctx.valueStack.back();
}

Value VirtualMachine::GetLocal(const ExecutionContext& ctx, const size_t index)
{
	if (ctx.callFrames.empty())
	{
		throw std::runtime_error("No call frame");
	}
	const auto& locals = ctx.callFrames.back().locals;
	if (index >= locals.size())
	{
		throw std::runtime_error("Local variable index out of range");
	}
	return locals[index];
}

void VirtualMachine::Run(ExecutionContext& ctx)
{
	while (ctx.isRunning)
	{
		if (ctx.callFrames.empty())
		{
			return;
		}
		CallFrame& currentFrame = ctx.callFrames.back();
		ctx.currentChunk = &currentFrame.function->chunk;
		ctx.instructionPointer = currentFrame.instructionPointer;

		if (ctx.instructionPointer >= ctx.currentChunk->code.size())
		{
			OpReturn(ctx);
			continue;
		}
		const auto opCode = static_cast<OpCode>(ReadByte(ctx));
		switch (opCode)
		{
		case OpCode::OP_CONSTANT:
			OpConstant(ctx);
			break;
		case OpCode::OP_RETURN:
			OpReturn(ctx);
			if (ctx.callFrames.empty())
			{
				return;
			}
			continue;
		case OpCode::OP_ADD:
			OpAdd(ctx);
			break;
		case OpCode::OP_SUB:
			OpSub(ctx);
			break;
		case OpCode::OP_MULTI:
			OpMulti(ctx);
			break;
		case OpCode::OP_DIV:
			OpDiv(ctx);
			break;
		case OpCode::OP_MOD:
			OpMod(ctx);
			break;
		case OpCode::OP_NEG:
			OpNeg(ctx);
			break;
		case OpCode::OP_AND:
			OpAnd(ctx);
			break;
		case OpCode::OP_OR:
			OpOr(ctx);
			break;
		case OpCode::OP_NOT:
			OpNot(ctx);
			break;
		case OpCode::OP_EQUAL:
			OpEqual(ctx);
			break;
		case OpCode::OP_NOT_EQUAL:
			OpNotEqual(ctx);
			break;
		case OpCode::OP_GREATER:
			OpGrater(ctx);
			break;
		case OpCode::OP_LESS:
			OpLess(ctx);
			break;
		case OpCode::OP_LESS_OR_EQUAL:
			OpLessOrEqual(ctx);
			break;
		case OpCode::OP_MORE_OR_EQUAL:
			OpMoreOrEqual(ctx);
			break;
		case OpCode::OP_SET_LOCAL_VAR:
			OpSetLocal(ctx);
			break;
		case OpCode::OP_GET_LOCAL_VAR:
			OpGetLocal(ctx);
			break;
		case OpCode::OP_JMP:
			OpJmp(ctx);
			break;
		case OpCode::OP_JMP_IF_FALSE:
			OpJmpIfFalse(ctx);
			break;
		case OpCode::OP_JMP_IF_LESS:
			OpJmpIfLess(ctx);
			break;
		case OpCode::OP_JMP_IF_GREATER:
			OpJmpIfGreater(ctx);
			break;
		case OpCode::OP_CALL:
			OpCall(ctx);
			continue;
		case OpCode::OP_FUNC:
			OpFunc(ctx);
			break;
		case OpCode::OP_CREATE_ARRAY:
			OpCreateArr(ctx);
			break;
		case OpCode::OP_GET_ELEMENT:
			OpGetEl(ctx);
			break;
		case OpCode::OP_SET_ELEMENT:
			OpSetEl(ctx);
			break;
		case OpCode::OP_GET_GLOBAL:
			OpGetGlobal(ctx);
			break;
		case OpCode::OP_SET_GLOBAL:
			OpSetGlobal(ctx);
			break;
		case OpCode::OP_POP:
			OpPop(ctx);
			break;
		case OpCode::OP_DUP:
			OpDup(ctx);
			break;
		case OpCode::OP_STRUCT_INIT:
			OpStructInit(ctx);
			break;
		case OpCode::OP_GET_FIELD:
			OpGetField(ctx);
			break;
		case OpCode::OP_SET_FIELD:
			OpSetField(ctx);
			break;
		case OpCode::OP_MAKE_CLOSURE:
			OpMakeClosure(ctx);
			break;
		default:
			std::cerr << "Unknown operation: " << static_cast<int>(opCode) << std::endl;
			return;
		}
		currentFrame.instructionPointer = ctx.instructionPointer;
	}
}

void VirtualMachine::OpConstant(ExecutionContext& ctx)
{
	const auto index = ReadByte(ctx);
	const auto constant = ctx.currentChunk->constants[index];
	Push(ctx, constant);
}

void VirtualMachine::OpReturn(ExecutionContext& ctx)
{
	Value returnValue;
	if (!ctx.valueStack.empty())
	{
		returnValue = Pop(ctx);
	}

	ctx.callFrames.pop_back();

	if (ctx.callFrames.empty())
	{
		if (!std::holds_alternative<std::monostate>(returnValue))
		{
			Push(ctx, returnValue);
		}
		return;
	}

	auto& callerFrame = ctx.callFrames.back();
	ctx.currentChunk = &callerFrame.function->chunk;
	ctx.instructionPointer = callerFrame.instructionPointer;

	if (!std::holds_alternative<std::monostate>(returnValue))
	{
		Push(ctx, returnValue);
	}
}

void VirtualMachine::OpAdd(ExecutionContext& ctx)
{
	const auto right = Pop(ctx);
	const auto left = Pop(ctx);

	Push(ctx, left + right);
}

void VirtualMachine::OpSub(ExecutionContext& ctx)
{
	const auto first = Pop(ctx);
	const auto second = Pop(ctx);

	Push(ctx, second - first);
}

void VirtualMachine::OpMulti(ExecutionContext& ctx)
{
	const auto first = Pop(ctx);
	const auto second = Pop(ctx);

	Push(ctx, first * second);
}

void VirtualMachine::OpDiv(ExecutionContext& ctx)
{
	const auto first = Pop(ctx);
	const auto second = Pop(ctx);

	Push(ctx, second / first);
}

void VirtualMachine::OpMod(ExecutionContext& ctx)
{
	const auto first = Pop(ctx);
	const auto second = Pop(ctx);

	Push(ctx, second % first);
}

void VirtualMachine::OpNeg(ExecutionContext& ctx)
{
	Push(ctx, -Pop(ctx));
}

void VirtualMachine::OpEqual(ExecutionContext& ctx)
{
	const auto rhs = Pop(ctx);
	const auto lhs = Pop(ctx);
	Push(ctx, lhs == rhs);
}

void VirtualMachine::OpNotEqual(ExecutionContext& ctx)
{
	const auto rhs = Pop(ctx);
	const auto lhs = Pop(ctx);
	Push(ctx, lhs != rhs);
}

void VirtualMachine::OpGrater(ExecutionContext& ctx)
{
	const auto rhs = Pop(ctx);
	const auto lhs = Pop(ctx);
	Push(ctx, lhs > rhs);
}

void VirtualMachine::OpLess(ExecutionContext& ctx)
{
	const auto rhs = Pop(ctx);
	const auto lhs = Pop(ctx);
	Push(ctx, lhs < rhs);
}

void VirtualMachine::OpLessOrEqual(ExecutionContext& ctx)
{
	const auto rhs = Pop(ctx);
	const auto lhs = Pop(ctx);
	Push(ctx, lhs <= rhs);
}

void VirtualMachine::OpMoreOrEqual(ExecutionContext& ctx)
{
	const auto rhs = Pop(ctx);
	const auto lhs = Pop(ctx);
	Push(ctx, lhs >= rhs);
}

void VirtualMachine::OpAnd(ExecutionContext& ctx)
{
	const auto rhs = Pop(ctx);
	const auto lhs = Pop(ctx);
	Push(ctx, lhs && rhs);
}

void VirtualMachine::OpOr(ExecutionContext& ctx)
{
	const auto rhs = Pop(ctx);
	const auto lhs = Pop(ctx);
	Push(ctx, lhs || rhs);
}

void VirtualMachine::OpNot(ExecutionContext& ctx)
{
	const auto val = Pop(ctx);
	Push(ctx, !val);
}

void VirtualMachine::OpSetLocal(ExecutionContext& ctx)
{
	const auto index = ReadByte(ctx);
	const auto value = Pop(ctx);

	if (ctx.callFrames.empty())
	{
		throw std::runtime_error("No call frame");
	}

	auto& locals = ctx.callFrames.back().locals;
	if (index >= locals.size())
	{
		throw std::runtime_error("Local variable index out of range");
	}
	locals[index] = value;
}

void VirtualMachine::OpGetLocal(ExecutionContext& ctx)
{
	const auto index = ReadByte(ctx);
	if (ctx.callFrames.empty())
	{
		throw std::runtime_error("No call frame");
	}

	const auto& locals = ctx.callFrames.back().locals;
	if (index >= locals.size())
	{
		throw std::runtime_error("Local variable index out of range");
	}
	Push(ctx, locals[index]);
}

void VirtualMachine::OpJmp(ExecutionContext& ctx)
{
	const auto address = static_cast<uint16_t>(ReadByte(ctx)) << BYTE_SIZE | ReadByte(ctx);
	ctx.instructionPointer = address;
}

void VirtualMachine::OpJmpIfFalse(ExecutionContext& ctx)
{
	const auto address = static_cast<uint16_t>(ReadByte(ctx)) << BYTE_SIZE | ReadByte(ctx);
	const auto condition = Pop(ctx);

	if (!condition)
	{
		ctx.instructionPointer = address;
	}
}

void VirtualMachine::OpJmpIfLess(ExecutionContext& ctx)
{
	const auto address = static_cast<uint16_t>(ReadByte(ctx)) << BYTE_SIZE | ReadByte(ctx);
	const auto value = Pop(ctx);

	if (!std::holds_alternative<int64_t>(value) && !std::holds_alternative<double>(value))
	{
		throw std::runtime_error("Condition for JMP_IF_LESS must be numeric");
	}

	if (std::holds_alternative<int64_t>(value) && std::get<int64_t>(value) < 0
		|| std::holds_alternative<double>(value) && std::get<double>(value) < 0)
	{
		ctx.instructionPointer = address;
	}
}
void VirtualMachine::OpJmpIfGreater(ExecutionContext& ctx)
{
	const auto address = static_cast<uint16_t>(ReadByte(ctx)) << BYTE_SIZE | ReadByte(ctx);
	const auto value = Pop(ctx);

	if (!std::holds_alternative<int64_t>(value) && !std::holds_alternative<double>(value))
	{
		throw std::runtime_error("Condition for JMP_IF_GREATER must be numeric");
	}

	if (std::holds_alternative<int64_t>(value) && std::get<int64_t>(value) > 0
		|| std::holds_alternative<double>(value) && std::get<double>(value) > 0)
	{
		ctx.instructionPointer = address;
	}
}

void VirtualMachine::OpCall(ExecutionContext& ctx)
{
	const auto argCount = ReadByte(ctx);
	const auto callee = Peek(ctx, argCount);
	if (IsNativeFunction(callee))
	{
		auto native = AsNativeFunction(callee);
		if (argCount != native->arity)
		{
			throw std::runtime_error(native->name + " error: Expected " + std::to_string(native->arity) + " arguments but got " + std::to_string(argCount));
		}

		std::vector<Value> args;
		for (int i = 0; i < argCount; ++i)
		{
			args.push_back(ctx.valueStack[ctx.valueStack.size() - argCount + i]);
		}

		for (int i = 0; i <= argCount; ++i)
		{
			ctx.valueStack.pop_back();
		}
		const auto result = native->func(args);
		Push(ctx, result);

		CallFrame& currentFrame = ctx.callFrames.back();
		currentFrame.instructionPointer = ctx.instructionPointer;
		return;
	}
	std::shared_ptr<Function> function = nullptr;
	std::vector<Value> captures;

	if (IsClosure(callee))
	{
		const auto closure = AsClosure(callee);
		function = closure->function;
		captures = closure->captures;
	}
	else if (IsFunction(callee))
	{
		function = AsFunc(callee);
	}
	else
	{
		throw std::runtime_error("VM Error: Can only call functions or closures");
	}

	if (argCount != function->arity)
	{
		throw std::runtime_error(function->name->str + " error: Expected " + std::to_string(function->arity) + " arguments but got " + std::to_string(argCount));
	}

	CallFrame frame(function);
	int localIdx = 0;
	for (const auto& cap : captures)
	{
		frame.locals[localIdx++] = cap;
	}

	for (int i = 0; i < argCount; i++)
	{
		const auto arg = ctx.valueStack[ctx.valueStack.size() - argCount + i];
		frame.locals[localIdx++] = arg;
	}
	for (int i = 0; i <= argCount; i++)
	{
		ctx.valueStack.pop_back();
	}

	CallFrame& currentFrame = ctx.callFrames.back();
	currentFrame.instructionPointer = ctx.instructionPointer;

	ctx.callFrames.push_back(frame);
	ctx.instructionPointer = 0;
}

void VirtualMachine::OpFunc(ExecutionContext& ctx)
{
	const auto funcIndex = ReadByte(ctx);
	if (funcIndex >= ctx.currentChunk->constants.size())
	{
		throw std::runtime_error("Function index out of range");
	}

	const auto funcValue = ctx.currentChunk->constants[funcIndex];

	if (!IsFunction(funcValue))
	{
		throw std::runtime_error("Expected function constant");
	}

	Push(ctx, funcValue);
}

void VirtualMachine::OpCreateArr(ExecutionContext& ctx)
{
	const auto count = ReadByte(ctx);
	auto array = std::make_shared<Array>(count);

	for (int i = count - 1; i >= 0; --i)
	{
		array->elements[i] = Pop(ctx);
	}
	Push(ctx, array);
}

void VirtualMachine::OpGetEl(ExecutionContext& ctx)
{
	const auto indexVal = Pop(ctx);
	const auto arrayVal = Pop(ctx);

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
	Push(ctx, arr->elements[idx]);
}

void VirtualMachine::OpSetEl(ExecutionContext& ctx)
{
	const auto indexVal = Pop(ctx);
	const auto arrayVal = Pop(ctx);
	const auto value = Pop(ctx);

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

void VirtualMachine::OpGetGlobal(ExecutionContext& ctx)
{
	const uint8_t nameIndex = ReadByte(ctx);
	const auto& name = ctx.currentChunk->constants[nameIndex];
	if (!IsString(name))
	{
		throw std::runtime_error("Expected string constant for global name");
	}
	const auto nameStr = AsString(name);

	std::shared_lock lock(m_globalsMutex);
	const auto it = m_globals.find(nameStr);
	if (it == m_globals.end())
	{
		throw std::runtime_error("Undefined global variable: " + nameStr);
	}
	Push(ctx, it->second);
}

void VirtualMachine::OpSetGlobal(ExecutionContext& ctx)
{
	const auto nameIndex = ReadByte(ctx);
	const auto& name = ctx.currentChunk->constants[nameIndex];
	if (!IsString(name))
	{
		throw std::runtime_error("Expected string constant for global name");
	}
	const auto nameStr = AsString(name);
	const auto value = Pop(ctx);

	std::unique_lock lock(m_globalsMutex);
	m_globals[nameStr] = value;
}

Value VirtualMachine::Peek(ExecutionContext& ctx, const int distance)
{
	if (distance >= static_cast<int>(ctx.valueStack.size()))
	{
		throw std::runtime_error("Stack index out of range");
	}
	return ctx.valueStack[ctx.valueStack.size() - 1 - distance];
}

uint16_t VirtualMachine::ReadShort(ExecutionContext& ctx)
{
	return static_cast<uint16_t>(ReadByte(ctx)) << BYTE_SIZE | ReadByte(ctx);
}

void VirtualMachine::Push(ExecutionContext& ctx, const Value& value)
{
	ctx.valueStack.push_back(value);
}

void VirtualMachine::OpPop(ExecutionContext& ctx)
{
	Pop(ctx);
}

void VirtualMachine::OpDup(ExecutionContext& ctx)
{
	if (ctx.valueStack.empty())
	{
		throw std::runtime_error("Const stack is empty");
	}
	const auto top = ctx.valueStack.back();
	Push(ctx, top);
}

void VirtualMachine::OpStructInit(ExecutionContext& ctx)
{
	const auto nameIdx = ReadByte(ctx);
	const auto structName = AsString(ctx.currentChunk->constants[nameIdx]);

	auto inst = std::make_shared<Instance>(structName);
	Push(ctx, inst);
}

void VirtualMachine::OpGetField(ExecutionContext& ctx)
{
	const auto nameIdx = ReadByte(ctx);
	const auto fieldName = AsString(ctx.currentChunk->constants[nameIdx]);

	const auto strct = Pop(ctx);
	if (!IsInstance(strct))
	{
		throw std::runtime_error("Cannot get field from non-instance object");
	}

	const auto instance = AsInstance(strct);
	auto it = instance->fields.find(fieldName);
	if (it == instance->fields.end())
	{
		Push(ctx, {});
	}
	else
	{
		Push(ctx, it->second);
	}
}

void VirtualMachine::OpSetField(ExecutionContext& ctx)
{
	const auto nameIdx = ReadByte(ctx);
	const auto fieldName = AsString(ctx.currentChunk->constants[nameIdx]);

	const auto val = Pop(ctx);
	const auto strct = Pop(ctx);

	if (!IsInstance(strct))
	{
		throw std::runtime_error("Cannot set field on non-instance object");
	}
	auto instance = AsInstance(strct);
	instance->fields[fieldName] = val;
	Push(ctx, val);
}

void VirtualMachine::OpMakeClosure(ExecutionContext& ctx)
{
	const auto captureCount = ReadByte(ctx);
	std::vector<Value> captures(captureCount);

	for (int i = captureCount - 1; i >= 0; i--)
	{
		captures[i] = Pop(ctx);
	}

	const auto funcValue = Pop(ctx);
	if (!IsFunction(funcValue))
	{
		throw std::runtime_error("VM Error: Expected function for OP_MAKE_CLOSURE");
	}
	auto closure = std::make_shared<Closure>(AsFunc(funcValue), std::move(captures));
	Push(ctx, closure);
}

Value VirtualMachine::Pop(ExecutionContext& ctx)
{
	if (ctx.valueStack.empty())
	{
		throw std::runtime_error("Const stack is empty");
	}
	const auto top = ctx.valueStack.back();
	ctx.valueStack.pop_back();
	return top;
}

uint8_t VirtualMachine::ReadByte(ExecutionContext& ctx)
{
	return ctx.currentChunk->code[ctx.instructionPointer++];
}

void VirtualMachine::RegisterNativeLib()
{
	// base
	RegisterNativeFunction("panic", 1, NativePanic);
	RegisterNativeFunction("print", 1, NativePrint);
	RegisterNativeFunction("read", 0, NativeRead);
	RegisterNativeFunction("time", 0, NativeTime);
	RegisterNativeFunction("len", 1, NativeLen);
	RegisterNativeFunction("random", 2, NativeRandomInt);
	RegisterNativeFunction("to_str", 1, NativeToString);
	RegisterNativeFunction("to_int", 1, NativeToInt);
	RegisterNativeFunction("append", 2, NativeAppend);
	RegisterNativeFunction("concat", 2, NativeConcat);

	// file
	RegisterNativeFunction("file_read", 1, NativeFileRead);
	RegisterNativeFunction("file_write", 2, NativeFileWrite);
	RegisterNativeFunction("file_exists", 1, NativeFileExists);

	// sql
	RegisterNativeFunction("db_connect", 4, NativeDBConnect);
	RegisterNativeFunction("db_execute", 2, NativeDBExecute);
	RegisterNativeFunction("db_disconnect", 1, NativeDBDisconnect);

	// http
	RegisterNativeFunction("http_init", 0, NativeHttpInit);
	RegisterNativeFunction("http_setopt", 3, NativeHttpSetOpt);
	RegisterNativeFunction("http_send", 1, NativeHttpSend);
	RegisterNativeFunction("http_status", 1, NativeHttpGetStatus);
	RegisterNativeFunction("http_response", 1, NativeHttpGetResponse);
	RegisterNativeFunction("http_cleanup", 1, NativeHttpCleanup);

	RegisterNativeFunction("http_send_async", 1, NativeHttpSendAsync);
	RegisterNativeFunction("http_is_ready", 1, NativeHttpIsReady);
	RegisterNativeFunction("http_await", 1, NativeHttpAwait);

	RegisterNativeFunction("http_server_start", 1, NativeHttpServerStart);
	RegisterNativeFunction("http_server_stop", 1, NativeHttpServerStop);
	RegisterNativeFunction("http_server_accept", 1, NativeHttpServerAccept);
	RegisterNativeFunction("http_request_method", 1, NativeHttpRequestMethod);
	RegisterNativeFunction("http_request_path", 1, NativeHttpRequestPath);
	RegisterNativeFunction("http_request_header", 2, NativeHttpRequestHeader);
	RegisterNativeFunction("http_request_body", 1, NativeHttpRequestBody);
	RegisterNativeFunction("http_request_respond", 4, NativeHttpRequestRespond);

	// threads
	RegisterNativeFunction("thread_spawn", 2, [this](const std::vector<Value>& args) -> Value {
		const auto& funcValue = args[0];
		const auto& argsArr = args[1];

		if (!IsArray(argsArr))
		{
			throw std::runtime_error("thread_spawn: args must be an array");
		}
		const auto arr = AsArray(argsArr);
		std::shared_ptr<Function> function;
		std::vector<Value> captures;

		if (IsClosure(funcValue))
		{
			const auto closure = AsClosure(funcValue);
			function = closure->function;
			captures = closure->captures;
		}
		else if (IsFunction(funcValue))
		{
			function = AsFunc(funcValue);
		}
		else
		{
			throw std::runtime_error("thread_spawn: first argument must be a function");
		}
		if (function->arity != arr->elements.size())
		{
			throw std::runtime_error("thread_spawn: incorrect number of arguments");
		}

		ExecutionContext newCtx;
		CallFrame frame(function);

		int localIdx = 0;
		for (const auto& capt : captures)
		{
			frame.locals[localIdx++] = capt;
		}
		for (const auto& arg : arr->elements)
		{
			frame.locals[localIdx++] = arg;
		}
		newCtx.callFrames.push_back(frame);
		newCtx.currentChunk = &function->chunk;
		newCtx.instructionPointer = 0;

		int64_t handle;
		{
			auto tState = std::make_unique<ThreadState>();

			std::lock_guard lock(g_threadsMutex);
			handle = g_nextThreadId++;
			g_threads[handle] = std::move(tState);
		}
		g_threads[handle]->worker = std::thread([this, newCtx, handle]() mutable {
			try
			{
				this->Run(newCtx);

				std::lock_guard lock(g_threadsMutex);
				if (!newCtx.valueStack.empty())
				{
					g_threads[handle]->result = newCtx.valueStack.back();
				}
			}
			catch (const std::exception& e)
			{
				std::lock_guard lock(g_threadsMutex);
				g_threads[handle]->errorMsg = e.what();
			}
			g_threads[handle]->isDone = true;
		});
		return handle;
	});

	RegisterNativeFunction("thread_join", 1, [](const std::vector<Value>& args) -> Value {
		const auto handle = AsInt(args[0]);

		ThreadState* tState = nullptr;
		{
			std::lock_guard lock(g_threadsMutex);
			if (!g_threads.contains(handle))
			{
				throw std::runtime_error("Invalid thread handle");
			}
			tState = g_threads[handle].get();
		}
		if (tState->worker.joinable())
		{
			tState->worker.join();
		}
		if (!tState->errorMsg.empty())
		{
			throw std::runtime_error("Thread panicked: " + tState->errorMsg);
		}
		auto res = tState->result;
		{
			std::lock_guard lock(g_threadsMutex);
			g_threads.erase(handle);
		}
		return res;
	});
}
