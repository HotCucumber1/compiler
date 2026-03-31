#pragma once

#include "StringPool/StringPool.h"
#include "Type/Chunk.h"

#include <memory>
#include <vector>

class VirtualMachine
{
public:
	static constexpr short STACK_SIZE = 256;

	VirtualMachine() = default;

	void Interpret(std::unique_ptr<Chunk> chunk);

private:
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

	void OpCall();

	void OpFunc();

	void Push(const Value& value);

	Value Pop();

	Value Peek(int distance) const;

	uint8_t ReadByte();

	uint16_t ReadShort();

	Value GetStackTop() const;

	Value GetLocal(size_t index) const;

private:
	friend struct VMAccessor;

	size_t m_instructionPointer = 0;
	StringPool m_stringPool; // TODO применить стринг пул
	Chunk* m_currentChunk{};

	std::vector<CallFrame> m_callFrames;
	std::vector<Value> m_valueStack;

	// std::stack<Value> m_constLiterals;
	// std::vector<Value> m_localVars;
};