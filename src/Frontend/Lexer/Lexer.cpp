#include "Lexer.h"
#include "Grammar/Identifier.h"
#include "Grammar/Keywords.h"
#include "Grammar/Number.h"
#include "Grammar/SpecialChar.h"
#include "Grammar/String.h"

Lexer::Lexer(const std::string& str)
	: m_reader(str)
{
}

Lexer::Lexer(const std::istream& input)
	: m_reader(input)
{
}

Token Lexer::GetToken()
{
	if (IsEmpty())
	{
		return {
			m_reader.GetCurrentPosition(),
			m_reader.GetCurrentLine(),
			TokenType::ERROR,
		};
	}

	const char ch = m_reader.GetNext();

	if (IsChar(ch))
	{
		return CheckKeywords(Identifier());
	}
	if (IsDigit(ch))
	{
		return Number();
	}
	if (IsQuote(ch))
	{
		return String();
	}
	if (IsSpecialChar(ch))
	{
		return SpecialChar();
	}

	return {
		m_reader.GetCurrentPosition(),
		m_reader.GetCurrentLine(),
		TokenType::ERROR,
	};
}

bool Lexer::IsEmpty()
{
	m_reader.SkipSpaces();
	return m_reader.IsEmpty();
}

Token Lexer::Identifier()
{
	const auto startPos = m_reader.GetCurrentPosition();
	m_reader.ResetBuffer();

	if (!IdentifierGrammar(m_reader))
	{
		return {
			startPos,
			m_reader.GetCurrentLine(),
			TokenType::ERROR,
		};
	}

	return {
		startPos,
		m_reader.GetCurrentLine(),
		TokenType::IDENTIFIER,
		m_reader.GetBuffer(),
	};
}

Token Lexer::Number()
{
	const auto startPos = m_reader.GetCurrentPosition();
	m_reader.ResetBuffer();
	bool isInteger = false;

	if (!NumberGrammar(m_reader, isInteger))
	{
		return {
			startPos,
			m_reader.GetCurrentLine(),
			TokenType::ERROR,
		};
	}

	return {
		startPos,
		m_reader.GetCurrentLine(),
		isInteger
			? TokenType::INT
			: TokenType::FLOAT,
		m_reader.GetBuffer(),
	};
}

Token Lexer::String()
{
	const auto startPos = m_reader.GetCurrentPosition();
	m_reader.ResetBuffer();

	if (!StringGrammar(m_reader))
	{
		return {
			startPos,
			m_reader.GetCurrentLine(),
			TokenType::ERROR,
		};
	}

	return {
		startPos,
		m_reader.GetCurrentLine(),
		TokenType::STRING,
		m_reader.GetBuffer(),
	};
}

Token Lexer::SpecialChar()
{
	const auto startPos = m_reader.GetCurrentPosition();
	m_reader.ResetBuffer();
	const auto tokenType = SpecialCharGrammar(m_reader);

	return {
		startPos,
		m_reader.GetCurrentLine(),
		tokenType,
		m_reader.GetBuffer(),
	};
}
