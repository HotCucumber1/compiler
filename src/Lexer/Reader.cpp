#include "Reader.h"

#include <iostream>

Reader::Reader(const std::string& string)
	: m_input(string)
{
}

Reader::Reader(const std::istream& input)
{
	m_input << input.rdbuf();
}

char Reader::GetCh()
{
	if (IsEmpty())
	{
		throw std::runtime_error("Failed to read from empty input");
	}
	const auto ch = static_cast<char>(m_input.get());

	if (m_toNextLine)
	{
		m_currentLine++;
		m_currentPosition = 1;
		m_toNextLine = false;
	}

	if (ch == '\n')
	{
		m_toNextLine = true;
	}
	else
	{
		m_currentPosition++;
		m_globalPosition++;
	}
	if (ch != '\n')
	{
		m_buffer += ch;
	}
	return ch;
}

void Reader::Seek(const size_t pos)
{
	const size_t dropLen = m_currentPosition - pos;
	if (dropLen <= m_buffer.size())
	{
		m_buffer.resize(m_buffer.size() - dropLen);
	}
	else
	{
		m_buffer.clear();
	}
	const auto offset = pos - m_currentPosition;
	m_currentPosition = pos;

	m_input.clear();
	m_input.seekg(static_cast<long>(offset), std::ios_base::cur);
}

char Reader::GetNext()
{
	return static_cast<char>(m_input.peek());
}

int Reader::GetCurrentPosition() const
{
	return m_currentPosition;
}

int Reader::GetCurrentLine() const
{
	return m_currentLine;
}

bool Reader::IsEmpty() const
{
	return m_input.eof();
}

void Reader::ResetBuffer()
{
	m_buffer.clear();
}

void Reader::SkipSpaces()
{
	while (m_input.good() and std::isspace(static_cast<char>(m_input.peek())))
	{
		const auto ch = static_cast<char>(m_input.get());
		if (ch == '\n')
		{
			m_currentLine++;
			m_currentPosition = 1;
		}
		else
		{
			m_currentPosition++;
			m_globalPosition++;
		}
	}
}

std::string Reader::GetBuffer() const
{
	return m_buffer;
}
