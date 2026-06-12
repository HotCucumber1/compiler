#include "json.h"
#include "../../../../lib/json.hpp"

Value NativeJsonToMap(const std::vector<Value>& args)
{
	std::string jsonStr = AsString(args[0]);

	if (jsonStr.size() >= 2 && jsonStr.front() == '"' && jsonStr.back() == '"')
	{
		jsonStr = jsonStr.substr(1, jsonStr.size() - 2);
	}

	size_t pos = 0;
	while ((pos = jsonStr.find("\\\"", pos)) != std::string::npos)
	{
		jsonStr.replace(pos, 2, "\"");
		pos += 1;
	}
	auto parsedJson = nlohmann::json::parse(jsonStr, nullptr, false);

	if (parsedJson.is_discarded() || !parsedJson.is_object())
	{
		throw std::runtime_error("Invalid JSON format. Expected an object. Got string:\n" + jsonStr);
	}

	auto array = std::make_shared<Array>(parsedJson.size());
	int i = 0;

	for (auto it = parsedJson.begin(); it != parsedJson.end(); ++it)
	{
		auto kvInstance = std::make_shared<Instance>("KeyValue");
		kvInstance->fields["key"] = MakeString(it.key());

		if (it.value().is_string())
		{
			kvInstance->fields["value"] = MakeString(it.value().get<std::string>());
		}
		else
		{
			kvInstance->fields["value"] = MakeString(it.value().dump());
		}

		array->elements[i++] = kvInstance;
	}

	auto mapInstance = std::make_shared<Instance>("StringMap");
	mapInstance->fields["entries"] = array;

	return mapInstance;
}

Value NativeMapToJson(const std::vector<Value>& args)
{
	const auto& mapVal = args[0];
	if (!IsInstance(mapVal))
	{
		throw std::runtime_error("Expected StringMap instance");
	}

	const auto mapInstance = AsInstance(mapVal);

	if (!mapInstance->fields.contains("entries"))
	{
		return MakeString("{}");
	}

	const auto entriesVal = mapInstance->fields["entries"];
	if (!IsArray(entriesVal))
	{
		return MakeString("{}");
	}

	const auto array = AsArray(entriesVal);
	nlohmann::json jsonObj;

	for (const auto& elem : array->elements)
	{
		if (IsInstance(elem))
		{
			const auto kvInstance = AsInstance(elem);
			if (kvInstance->fields.contains("key") && kvInstance->fields.contains("value"))
			{
				std::string k = AsString(kvInstance->fields["key"]);
				std::string v = AsString(kvInstance->fields["value"]);
				jsonObj[k] = v;
			}
		}
	}

	return MakeString(jsonObj.dump());
}