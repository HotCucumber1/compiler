#pragma once

#include "StringPool.h"
#include "Value.h"

#include <memory>
#include <stack>
#include <vector>

struct Chunk
{
	std::vector<uint8_t> code;
	std::vector<Value> constants;
};

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

	void Push(const Value& value);

	Value Pop();

	uint8_t ReadByte();

private:
	StringPool m_stringPool; // TODO применить стринг пул
	std::unique_ptr<Chunk> m_currentChunk;
	size_t m_iPointer = 0;
	std::stack<Value> m_constLiterals;
	std::stack<Value> m_localVars;
};