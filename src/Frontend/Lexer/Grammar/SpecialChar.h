#pragma once
#include "../Reader.h"
#include "../../Common/Token.h"

inline TokenType SpecialCharGrammar(Reader& reader)
{
	const auto it = SPECIAL_CHARS.find(reader.GetNext());

	reader.GetCh();
	if (reader.IsEmpty())
	{
		return it->second;
	}

	const auto doubledIt = SPECIAL_CHARS_DOUBLE.find(it->first);
	if (doubledIt == SPECIAL_CHARS_DOUBLE.end() || doubledIt->second.first != reader.GetNext())
	{
		return it->second;
	}

	reader.GetCh();
	return doubledIt->second.second;
}

inline bool IsSpecialChar(const char c)
{
	return SPECIAL_CHARS.contains(c);
}
