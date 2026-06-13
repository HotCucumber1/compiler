#pragma once
#include "../../lib/json.hpp"
#include <string>
#include <unordered_map>

enum class DataType
{
	INT,
	FLOAT,
	BOOL,
	STRING,
	INSTANCE,
	VOID,
	FUNC,
	UNKNOWN
};

const std::unordered_map<DataType, std::string> DATA_TYPE_NAMES = {
	{ DataType::INT, "int" },
	{ DataType::FLOAT, "float" },
	{ DataType::BOOL, "bool" },
	{ DataType::STRING, "string" },
	{ DataType::INSTANCE, "struct instance" },
	{ DataType::VOID, "void" },
	{ DataType::FUNC, "func" },
	{ DataType::UNKNOWN, "unknown" },
};

NLOHMANN_JSON_SERIALIZE_ENUM(
	DataType,
	{
		{ DataType::UNKNOWN, "UNKNOWN" },
		{ DataType::INT, "INT" },
		{ DataType::FLOAT, "FLOAT" },
		{ DataType::BOOL, "BOOL" },
		{ DataType::STRING, "STRING" },
		{ DataType::VOID, "VOID" },
		{ DataType::INSTANCE, "INSTANCE" },
		{ DataType::FUNC, "FUNC" },
	});
