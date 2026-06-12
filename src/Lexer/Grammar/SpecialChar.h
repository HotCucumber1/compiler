#pragma once
#include "../../Common/Token.h"
#include "../Reader.h"

inline TokenType SpecialCharGrammar(Reader& reader)
{
	const char firstChar = reader.GetNext();
	const auto it = SPECIAL_CHARS.find(firstChar);

	reader.GetCh();
	if (!reader.IsEmpty())
	{
		const auto doubledIt = SPECIAL_CHARS_DOUBLE.find(firstChar);
		if (doubledIt != SPECIAL_CHARS_DOUBLE.end())
		{
			const char secondChar = reader.GetNext();
			const auto secondIt = doubledIt->second.find(secondChar);

			if (secondIt != doubledIt->second.end())
			{
				reader.GetCh();
				return secondIt->second;
			}
		}
	}
	if (it != SPECIAL_CHARS.end())
	{
		return it->second;
	}
	return TokenType::ERROR;
}

inline bool IsSpecialChar(const char c)
{
	return SPECIAL_CHARS.contains(c) || SPECIAL_CHARS_DOUBLE.contains(c);
}
