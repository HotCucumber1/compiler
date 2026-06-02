#pragma once
#include <unordered_map>
#include <string>

enum class DataType
{
	INT,
	FLOAT,
	BOOL,
	STRING,
	INSTANCE,
	VOID,
	UNKNOWN
};

const std::unordered_map<DataType, std::string> DATA_TYPE_NAMES = {
	{ DataType::INT, "int" },
	{ DataType::FLOAT, "float" },
	{ DataType::BOOL, "bool" },
	{ DataType::STRING, "string" },
	{ DataType::INSTANCE, "struct instance" },
	{ DataType::VOID, "void" },
	{ DataType::UNKNOWN, "unknown" },
};
