#pragma once
#include "Grammar.h"

class GrammarReader
{
	struct LineContent
	{
		std::string lhs;
		std::string rhs;
		std::string type;
	};

public:
	static Grammar Read(const std::string& srcDir);

private:
	static LineContent ParseLine(const std::string& line);
};
