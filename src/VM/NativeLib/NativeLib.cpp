#include "NativeLib.h"

#include <fstream>
#include <sstream>

Value NativePrint(const std::vector<Value>& args)
{
	for (const auto& arg : args)
	{
		PrintValue(arg);
	}
	return {};
}

Value NativeRead(const std::vector<Value>&)
{
	return Read();
}

Value NativeFileRead(const std::vector<Value>& args)
{
	const std::string path = AsString(args[0]);
	std::ifstream file(path);

	if (!file.is_open())
	{
		return {};
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	return MakeString(buffer.str());
}

Value NativeFileWrite(const std::vector<Value>& args)
{
	const auto path = AsString(args[0]);
	const auto content = AsString(args[1]);

	std::ofstream file(path);
	if (!file.is_open())
	{
		return false;
	}

	file << content;
	return true;
}

Value NativeFileExists(const std::vector<Value>& args)
{
	const auto path = AsString(args[0]);
	const std::ifstream file(path);
	return file.good();
}

Value NativePanic(const std::vector<Value>& args)
{
	const std::string message = AsString(args[0]);
	throw std::runtime_error("Program panic: " + message);
}