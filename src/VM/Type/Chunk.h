#pragma once
#include "Value.h"

#include <unordered_map>
#include <vector>

struct Chunk
{
	size_t localsCount;
	std::vector<uint8_t> code;
	std::vector<Value> constants;

	std::unordered_map<std::string, size_t> labels;
	std::vector<std::pair<size_t, std::string>> pendingJumps;
};

struct Function final : Object
{
	int arity;
	Chunk chunk{};
	String* name;

	explicit Function(String* name, const int arity)
		: Object(ObjectType::FUNC)
		, arity(arity)
		, name(name)
	{
	}
};

using FunctionPtr = std::shared_ptr<Function>;

struct CallFrame
{
	FunctionPtr function;
	size_t instructionPointer = 0;
	std::vector<Value> locals;

	explicit CallFrame(const FunctionPtr& function)
		: function(function)
		, locals(function->chunk.localsCount)
	{
	}
};
