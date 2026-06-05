#pragma once
#include "Type/Value.h"

struct ExecutionContext
{
	std::vector<Value> valueStack;
	std::vector<CallFrame> callFrames;
	Chunk* currentChunk = nullptr;
	size_t instructionPointer = 0;
	bool isRunning = true;
};
