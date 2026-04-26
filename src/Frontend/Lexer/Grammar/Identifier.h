#pragma once
#include "../Reader.h"

inline bool IsChar(const char ch)
{
	return ch == '_' or std::isalpha(ch);
}

inline bool IdentifierChar(Reader& reader)
{
	const auto ch = reader.GetNext();
	return IsChar(ch);
}

inline bool IdentifierPart(Reader& reader)
{
	if (IdentifierChar(reader))
	{
		reader.GetCh();
		return true;
	}

	return std::isdigit(reader.GetCh());
}

inline bool IdentifierTail(Reader& reader)
{
	if (reader.IsEmpty())
	{
		return true;
	}

	const auto pos = reader.GetCurrentPosition();
	if (!IdentifierPart(reader))
	{
		reader.Seek(pos);
		return true;
	}

	return IdentifierTail(reader);
}

inline bool IdentifierGrammar(Reader& reader)
{
	if (reader.IsEmpty())
	{
		return false;
	}

	return IdentifierChar(reader) && IdentifierTail(reader);
}