#pragma once

#include "StringPool/StringPool.h"
#include "Type/Chunk.h"
#include "Type/NativeFunction.h"
#include "ExecutionContext.h"

#include <memory>
#include <shared_mutex>

class VirtualMachine
{
public:
	static constexpr short STACK_SIZE = 256;

	VirtualMachine();

	void Interpret(const std::unique_ptr<Chunk>& chunk);

	void Run(ExecutionContext& ctx);

private:
	void RegisterNativeFunction(const std::string& name, int arity, const NativeFunction::NativeFunc& func);

	static void OpConstant(ExecutionContext& ctx);

	static void OpReturn(ExecutionContext& ctx);

	static void OpAdd(ExecutionContext& ctx);

	static void OpSub(ExecutionContext& ctx);

	static void OpMulti(ExecutionContext& ctx);

	static void OpDiv(ExecutionContext& ctx);

	static void OpMod(ExecutionContext& ctx);

	static void OpNeg(ExecutionContext& ctx);

	static void OpEqual(ExecutionContext& ctx);

	static void OpNotEqual(ExecutionContext& ctx);

	static void OpGrater(ExecutionContext& ctx);

	static void OpLess(ExecutionContext& ctx);

	static void OpLessOrEqual(ExecutionContext& ctx);

	static void OpMoreOrEqual(ExecutionContext& ctx);

	static void OpAnd(ExecutionContext& ctx);

	static void OpOr(ExecutionContext& ctx);

	static void OpNot(ExecutionContext& ctx);

	static void OpSetLocal(ExecutionContext& ctx);

	static void OpGetLocal(ExecutionContext& ctx);

	static void OpJmp(ExecutionContext& ctx);

	static void OpJmpIfFalse(ExecutionContext& ctx);

	static void OpJmpIfLess(ExecutionContext& ctx);

	static void OpJmpIfGreater(ExecutionContext& ctx);

	static void OpCall(ExecutionContext& ctx);

	static void OpFunc(ExecutionContext& ctx);

	static void OpCreateArr(ExecutionContext& ctx);

	static void OpGetEl(ExecutionContext& ctx);

	static void OpSetEl(ExecutionContext& ctx);

	static void Push(ExecutionContext& ctx, const Value& value);

	static void OpPop(ExecutionContext& ctx);

	static void OpDup(ExecutionContext& ctx);

	static void OpStructInit(ExecutionContext& ctx);

	static void OpGetField(ExecutionContext& ctx);

	static void OpSetField(ExecutionContext& ctx);

	static void OpMakeClosure(ExecutionContext& ctx);

	static Value Pop(ExecutionContext& ctx);

	static Value Peek(ExecutionContext& ctx, int distance) ;

	static uint8_t ReadByte(ExecutionContext& ctx);

	static uint16_t ReadShort(ExecutionContext& ctx);

	static Value GetStackTop(ExecutionContext& ctx) ;

	static Value GetLocal(const ExecutionContext& ctx, size_t index) ;

	void OpGetGlobal(ExecutionContext& ctx);

	void OpSetGlobal(ExecutionContext& ctx);

	void RegisterNativeLib();

private:
	friend struct VMAccessor;

	// size_t m_instructionPointer = 0;
	// StringPool m_stringPool; // TODO применить стринг пул
	// Chunk* m_currentChunk{};

	// std::vector<CallFrame> m_callFrames;
	// std::vector<Value> m_valueStack;

	std::unordered_map<std::string, Value> m_globals;
	mutable std::shared_mutex m_globalsMutex;
};
