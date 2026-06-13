#include "ai_resp.h"
#include "../../../../lib/json.hpp"

Value NativeAiParseResponse(const std::vector<Value>& args)
{
	std::string jsonStr = AsString(args[0]);

	try
	{
		auto parsedJson = nlohmann::json::parse(jsonStr);
		if (!parsedJson.contains("choices") || !parsedJson["choices"].is_array() || parsedJson["choices"].empty())
		{
			return MakeString("Ошибка: В ответе нет поля choices.");
		}

		const auto contentStr = parsedJson["choices"][0]["message"]["content"].get<std::string>();
		return MakeString(contentStr);
	}
	catch (const nlohmann::json::exception& e)
	{
		return MakeString("Ошибка парсинга JSON: " + std::string(e.what()) + "\nСырой ответ: " + jsonStr);
	}
}
