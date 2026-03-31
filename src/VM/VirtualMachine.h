#pragma once

#include "StringPool/StringPool.h"
#include "Type/Chunk.h"
#include "Type/NativeFunction.h"

#include <memory>
#include <vector>

class VirtualMachine
{
public:
	static constexpr short STACK_SIZE = 256;

	VirtualMachine();

	void Interpret(std::unique_ptr<Chunk> chunk);

private:
	void RegisterNativeFunction(const std::string& name, int arity, const NativeFunction::NativeFunc& func);

	void Run();

	void OpConstant();

	void OpReturn();

	void OpAdd();

	void OpSub();

	void OpMulti();

	void OpDiv();

	void OpMod();

	void OpNeg();

	void OpSetLocal();

	void OpGetLocal();

	void OpJmp();

	void OpJmpIfFalse();

	void OpJmpIfLess();

	void OpJmpIfGreater();

	void OpCall();

	void OpFunc();

	void OpCreateArr();

	void OpGetEl();

	void OpSetEl();

	void Push(const Value& value);

	Value Pop();

	Value Peek(int distance) const;

	uint8_t ReadByte();

	uint16_t ReadShort();

	Value GetStackTop() const;

	Value GetLocal(size_t index) const;

	void OpGetGlobal();

	void OpSetGlobal();

private:
	friend struct VMAccessor;

	size_t m_instructionPointer = 0;
	StringPool m_stringPool; // TODO применить стринг пул
	Chunk* m_currentChunk{};

	std::vector<CallFrame> m_callFrames;
	std::vector<Value> m_valueStack;

	std::unordered_map<std::string, Value> m_globals;
	// std::stack<Value> m_constLiterals;
	// std::vector<Value> m_localVars;
};