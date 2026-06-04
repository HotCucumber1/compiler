#pragma once
#include "../../Common/Token.h"
#include "../Reader.h"

inline TokenType SpecialCharGrammar(Reader& reader)
{
	const char firstChar = reader.GetNext();
	const auto it = SPECIAL_CHARS.find(firstChar);

	reader.GetCh();
	if (reader.IsEmpty())
	{
		return it->second;
	}

	const auto doubledIt = SPECIAL_CHARS_DOUBLE.find(firstChar);
	if (doubledIt == SPECIAL_CHARS_DOUBLE.end())
	{
		return it->second;
	}

	const char secondChar = reader.GetNext();

	const auto secondIt = doubledIt->second.find(secondChar);
	if (secondIt == doubledIt->second.end())
	{
		return it->second;
	}

	reader.GetCh();
	return secondIt->second;
}

inline bool IsSpecialChar(const char c)
{
	return SPECIAL_CHARS.contains(c) || SPECIAL_CHARS_DOUBLE.contains(c);
}
