#pragma once
#include <sstream>
#include <string>
#include <vector>

class Reader
{
public:
	explicit Reader(const std::string& string);

	explicit Reader(const std::istream& input);

	char GetCh();

	void Seek(size_t pos);

	char GetNext();

	int GetCurrentPosition() const;

	int GetCurrentLine() const;

	bool IsEmpty() const;

	void ResetBuffer();

	void SkipSpaces();

	std::string GetBuffer() const;

private:
	int m_currentPosition = 1;
	int m_currentLine = 1;
	int m_globalPosition = 1;
	std::stringstream m_input;
	std::string m_buffer;
	bool m_toNextLine = false;
};
