#pragma once
#include "../VirtualMachine.h"

#include <memory>
#include <string>

class BytecodeParser
{
public:
	static std::unique_ptr<Chunk> Parse(const std::string& source);
};
