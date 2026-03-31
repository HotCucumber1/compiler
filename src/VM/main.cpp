#include "Parser/BytecodeParser.h"
#include "VirtualMachine.h"

#include <fstream>
#include <iostream>
#include <sstream>

std::string ReadFile(const std::string& filename);

int main(const int argc, char* argv[])
{
	try
	{
		if (argc < 2)
		{
			throw std::runtime_error("Need to specify bytecode source");
		}
		const auto bytecodeSource = ReadFile(argv[1]);

		auto chunk = BytecodeParser::Parse(bytecodeSource);

		VirtualMachine vm;
		vm.Interpret(std::move(chunk));
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
}

std::string ReadFile(const std::string& filename)
{
	std::ifstream file(filename);
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file '" + filename + "'");
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}
