#pragma once
#include "../Reader.h"

inline bool IsDigit(const char c)
{
	return c >= '0' && c <= '9';
}

inline bool NonZero(const char c)
{
	return c > '0' && c <= '9';
}

inline bool ExpTail(Reader& reader)
{
	if (reader.IsEmpty())
	{
		return true;
	}

	if (IsDigit(reader.GetNext()))
	{
		reader.GetCh();
		return ExpTail(reader);
	}

	return true;
}


inline bool ExpVal(Reader& reader)
{
	return !reader.IsEmpty() && NonZero(reader.GetCh()) && ExpTail(reader);
}

inline bool Exp(Reader& reader)
{
	if (reader.IsEmpty())
	{
		return false;
	}

	const auto sign = reader.GetCh();
	return (sign == '+' || sign == '-') && ExpVal(reader);
}

inline bool ExpPart(Reader& reader, bool& isInteger)
{
	if (reader.IsEmpty())
	{
		return true;
	}

	const auto ch = reader.GetNext();
	if (ch == 'e' || ch == 'E')
	{
		isInteger = false;
		reader.GetCh();
		return Exp(reader);
	}

	return true;
}

inline bool MantissaTail(Reader& reader)
{
	if (reader.IsEmpty())
	{
		return true;
	}

	if (IsDigit(reader.GetNext()))
	{
		reader.GetCh();
		return MantissaTail(reader);
	}

	return true;
}

inline bool Mantissa(Reader& reader)
{
	if (reader.IsEmpty())
	{
		return false;
	}

	return IsDigit(reader.GetCh()) && MantissaTail(reader);
}

inline bool OptMantissa(Reader& reader, bool& isInteger)
{
	if (reader.IsEmpty())
	{
		return true;
	}

	if (reader.GetNext() == '.')
	{
		isInteger = false;
		reader.GetCh();
		return Mantissa(reader);
	}

	return !IsDigit(reader.GetNext());
}

inline bool NumberTail(Reader& reader, bool& isInteger)
{
	if (reader.IsEmpty())
	{
		return true;
	}

	if (IsDigit(reader.GetNext()))
	{
		reader.GetCh();
		return NumberTail(reader, isInteger);
	}

	if (reader.GetNext() == '.')
	{
		isInteger = false;
		reader.GetCh();
		return Mantissa(reader);
	}

	return true;
}

inline bool Number(Reader& reader, bool& isInteger)
{
	if (reader.IsEmpty())
	{
		return false;
	}

	const auto ch = reader.GetNext();
	if (NonZero(ch))
	{
		reader.GetCh();
		return NumberTail(reader, isInteger);
	}

	if (ch == '0')
	{
		reader.GetCh();
		return OptMantissa(reader, isInteger);
	}

	return false;
}


inline bool NumberGrammar(Reader& reader, bool& isInteger)
{
	isInteger = true;
	return Number(reader, isInteger) && ExpPart(reader, isInteger);
}
