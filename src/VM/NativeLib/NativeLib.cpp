#include "NativeLib.h"

#include <chrono>
#include <fstream>
#include <random>
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

Value NativeTime(const std::vector<Value>&)
{
	const auto now = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
}

Value NativeLen(const std::vector<Value>& args)
{
	if (IsString(args[0]))
	{
		return static_cast<int64_t>(AsString(args[0]).size());
	}
	if (IsArray(args[0]))
	{
		return static_cast<int64_t>(AsArray(args[0])->elements.size());
	}
	throw std::runtime_error("len() supports only strings and arrays");
}

Value NativeRandomInt(const std::vector<Value>& args)
{
	const int min = AsInt(args[0]);
	const int max = AsInt(args[1]);

	thread_local std::random_device rd;
	thread_local std::mt19937 gen(rd());
	std::uniform_int_distribution<int64_t> distrib(min, max);

	return distrib(gen);
}

Value NativeToString(const std::vector<Value>& args)
{
	const auto& val = args[0];
	if (IsString(val))
	{
		return val;
	}
	if (std::holds_alternative<int64_t>(val))
	{
		return MakeString(std::to_string(std::get<int64_t>(val)));
	}
	if (std::holds_alternative<double>(val))
	{
		return MakeString(std::to_string(std::get<double>(val)));
	}
	if (std::holds_alternative<bool>(val))
	{
		return MakeString(std::get<bool>(val)
				? "true"
				: "false");
	}
	return MakeString("[Object]");
}

Value NativeToInt(const std::vector<Value>& args)
{
	const auto& val = args[0];
	if (std::holds_alternative<int64_t>(val))
	{
		return val;
	}
	if (std::holds_alternative<double>(val))
	{
		return static_cast<int64_t>(std::get<double>(val));
	}
	if (std::holds_alternative<bool>(val))
	{
		return std::get<bool>(val) ? 1 : 0;
	}
	if (std::holds_alternative<char>(val))
	{
		return std::get<char>(val);
	}

	if (IsString(val))
	{
		const std::string& str = AsString(val);
		try
		{
			return std::stoll(str);
		}
		catch (const std::invalid_argument&)
		{
			throw std::runtime_error("Cannot convert string '" + str + "' to int (invalid format).");
		}
		catch (const std::out_of_range&)
		{
			throw std::runtime_error("Cannot convert string '" + str + "' to int (out of range).");
		}
	}
	throw std::runtime_error("Unsupported type for to_int conversion.");
}
