#pragma once
#include "../Reader.h"

inline bool IsQuote(const char c)
{
	return c == '\'' || c == '"';
}

inline bool StringGrammar(Reader& reader)
{
	if (!IsQuote(reader.GetCh()) or reader.IsEmpty())
	{
		return false;
	}


	while (!IsQuote(reader.GetCh()))
	{
		if (reader.IsEmpty())
		{
			return false;
		}
	}

	return true;
}