#pragma once
#include "../Reader.h"

inline bool IsQuote(const char c)
{
	return c == '\'' || c == '"';
}

inline bool StringGrammar(Reader& reader, char expectedQuote)
{
	if (reader.IsEmpty())
	{
		return false;
	}
	if (reader.GetCh() != expectedQuote)
	{
		return false;
	}
	while (!reader.IsEmpty())
	{
		const char ch = reader.GetCh();
		if (ch == '\\' && !reader.IsEmpty())
		{
			reader.GetCh();
			continue;
		}
		if (ch == expectedQuote)
		{
			return true;
		}
	}
	return false;
}