#pragma once

#include "../../../lib/json.hpp"
#include "Object.h"
#include "Value.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct Chunk
{
	size_t localsCount = 0;
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

	explicit CallFrame(const FunctionPtr& function);
};

inline CallFrame::CallFrame(const FunctionPtr& function)
	: function(function)
	, locals(function->chunk.localsCount)
{
}

void to_json(nlohmann::json& jsonData, const Chunk& chunk);
void to_json(nlohmann::json& jsonData, const Value& val);

void from_json(const nlohmann::json& jsonData, Chunk& chunk);
void from_json(const nlohmann::json& jsonData, Value& val);

inline void to_json(nlohmann::json& jsonData, const Chunk& chunk)
{
	jsonData = nlohmann::json{
		{ "code", chunk.code },
		{ "constants", chunk.constants },
		{ "localsCount", chunk.localsCount },
	};
}

inline void from_json(const nlohmann::json& jsonData, Chunk& chunk)
{
	jsonData.at("code").get_to(chunk.code);
	jsonData.at("constants").get_to(chunk.constants);
	jsonData.at("localsCount").get_to(chunk.localsCount);
}

inline void to_json(nlohmann::json& jsonData, const Value& val)
{
	if (std::holds_alternative<std::monostate>(val))
	{
		jsonData = { { "type", "null" } };
	}
	else if (std::holds_alternative<int64_t>(val))
	{
		jsonData = { { "type", "int" }, { "val", std::get<int64_t>(val) } };
	}
	else if (std::holds_alternative<double>(val))
	{
		jsonData = { { "type", "float" }, { "val", std::get<double>(val) } };
	}
	else if (std::holds_alternative<bool>(val))
	{
		jsonData = { { "type", "bool" }, { "val", std::get<bool>(val) } };
	}
	else if (IsString(val))
	{
		jsonData = { { "type", "string" }, { "val", AsString(val) } };
	}
	else if (IsFunction(val))
	{
		const auto func = AsFunc(val);
		jsonData = {
			{ "type", "function" },
			{
				"val",
				{
					{ "name", func->name ? func->name->str : "" },
					{ "arity", func->arity },
					{ "chunk", func->chunk },
				},
			}
		};
	}
	else
	{
		throw std::runtime_error("CodeGen Error: Unsupported value type in constants!");
	}
}

inline void from_json(const nlohmann::json& jsonData, Value& val)
{
	auto type = jsonData.at("type").get<std::string>();

	if (type == "null")
	{
		val = std::monostate{};
	}
	else if (type == "int")
	{
		val = jsonData.at("val").get<int64_t>();
	}
	else if (type == "float")
	{
		val = jsonData.at("val").get<double>();
	}
	else if (type == "bool")
	{
		val = jsonData.at("val").get<bool>();
	}
	else if (type == "string")
	{
		val = MakeString(jsonData.at("val").get<std::string>());
	}
	else if (type == "function")
	{
		auto funcJson = jsonData.at("val");
		const auto nameStr = funcJson.at("name").get<std::string>();
		int arity = funcJson.at("arity").get<int>();

		auto funcNameObj = new String(nameStr);
		auto func = std::make_shared<Function>(funcNameObj, arity);

		funcJson.at("chunk").get_to(func->chunk);

		val = Value(func);
	}
}