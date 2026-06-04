#include "Lexer.h"

#include "Grammar/Identifier.h"
#include "Grammar/Keywords.h"
#include "Grammar/Number.h"
#include "Grammar/SpecialChar.h"
#include "Grammar/String.h"
#include <utility>

Lexer::Lexer(const std::string& str, std::string filePath)
	: m_reader(str)
	, m_filePath(std::move(filePath))
{
}

Lexer::Lexer(const std::istream& input, std::string filePath)
	: m_reader(input)
	, m_filePath(std::move(filePath))
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
			"",
			m_filePath,
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
		"",
		m_filePath,
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
			"",
			m_filePath,
		};
	}

	return {
		startPos,
		m_reader.GetCurrentLine(),
		TokenType::IDENTIFIER,
		m_reader.GetBuffer(),
		m_filePath,
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
			"",
			m_filePath,
		};
	}

	return {
		startPos,
		m_reader.GetCurrentLine(),
		isInteger
			? TokenType::INT
			: TokenType::FLOAT,
		m_reader.GetBuffer(),
		m_filePath,
	};
}

Token Lexer::String()
{
	const auto startPos = m_reader.GetCurrentPosition();
	m_reader.ResetBuffer();

	const auto expectedQuote = m_reader.GetNext();
	if (!StringGrammar(m_reader, expectedQuote))
	{
		return {
			startPos,
			m_reader.GetCurrentLine(),
			TokenType::ERROR,
			"",
			m_filePath,
		};
	}

	return {
		startPos,
		m_reader.GetCurrentLine(),
		TokenType::STRING,
		m_reader.GetBuffer(),
		m_filePath,
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
		m_filePath,
	};
}
