#pragma once

#include "StringPool.h"
#include "Value.h"

#include <memory>
#include <stack>
#include <vector>

struct Chunk
{
	size_t localsCount;
	std::vector<uint8_t> code;
	std::vector<Value> constants;
};

class VirtualMachine
{
public:
	static constexpr short STACK_SIZE = 256;

	explicit VirtualMachine(size_t localsCount);

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

	void Push(const Value& value);

	Value Pop();

	uint8_t ReadByte();

private:
	size_t m_iPointer = 0;
	StringPool m_stringPool; // TODO применить стринг пул
	std::unique_ptr<Chunk> m_currentChunk;

	std::stack<Value> m_constLiterals;
	std::vector<Value> m_localVars;
};