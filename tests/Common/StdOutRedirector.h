#pragma once
#include <iostream>
#include <sstream>

class StdOutRedirector
{
public:
	StdOutRedirector()
		: m_oldBuffer(std::cout.rdbuf(m_buffer.rdbuf()))
	{
	}
	~StdOutRedirector()
	{
		std::cout.rdbuf(m_oldBuffer);
	}

	std::string GetOutput() const
	{
		std::string out = m_buffer.str();
		if (!out.empty() && out.back() == '\n')
		{
			out.pop_back();
		}
		return out;
	}

private:
	std::stringstream m_buffer;
	std::streambuf* m_oldBuffer;
};
