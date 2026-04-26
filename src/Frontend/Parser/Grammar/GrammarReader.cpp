#include "GrammarReader.h"

#include <algorithm>
#include <fstream>
#include <sstream>

AstNodeType StringToAstType(const std::string& str);
std::string Trim(std::string s);
bool IsTerminal(const std::string& str);

Grammar GrammarReader::Read(const std::string& srcPath)
{
	Grammar g;
	std::ifstream file(srcPath);
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file '" + srcPath + "'");
	}
	std::string line;
	bool firstRule = true;

	while (std::getline(file, line))
	{
		if (line.empty() || line[0] == '#')
		{
			continue;
		}

		auto [lhsStr, rhsPart, typeStr] = ParseLine(line);

		RuleItem lhs = {
			lhsStr,
			false,
		};
		std::vector<RuleItem> rhs;

		std::stringstream ss(rhsPart);
		std::string word;
		while (ss >> word)
		{
			rhs.push_back({
				word,
				IsTerminal(word),
			});
		}

		const auto nodeType = StringToAstType(typeStr);

		g.AddRule(lhs, rhs, firstRule, nodeType);
		firstRule = false;
	}

	return g;
}

GrammarReader::LineContent GrammarReader::ParseLine(const std::string& line)
{
	const auto arrowPos = line.find("->");
	const auto colonPos = line.find(':');

	const auto lhs = Trim(line.substr(0, arrowPos));
	const auto rhs = (colonPos != std::string::npos)
		? Trim(line.substr(arrowPos + 2, colonPos - (arrowPos + 2)))
		: Trim(line.substr(arrowPos + 2));
	const auto type = (colonPos != std::string::npos)
		? Trim(line.substr(colonPos + 1))
		: "UNDEFINED";

	return {
		lhs,
		rhs,
		type,
	};
}

bool IsTerminal(const std::string& str)
{
	if (str.empty())
	{
		return false;
	}
	if (KEYWORDS.contains(str)
		|| SPECIAL_CHARS.contains(str[0])
		|| str == TOKEN_NAMES.at(TokenType::IDENTIFIER)
		|| str == TOKEN_NAMES.at(TokenType::INT)
		|| str == TOKEN_NAMES.at(TokenType::FLOAT)
		|| str == TOKEN_NAMES.at(TokenType::STRING))
	{
		return true;
	}
	if (str.length() == 2)
	{
		auto it = SPECIAL_CHARS_DOUBLE.find(str[0]);
		if (it != SPECIAL_CHARS_DOUBLE.end() && it->second.first == str[1])
		{
			return true;
		}
	}
	return false;
}

AstNodeType StringToAstType(const std::string& str)
{
	return AST_NODE_TYPES.contains(str)
		? AST_NODE_TYPES.at(str)
		: AstNodeType::UNDEFINED;
}

std::string Trim(std::string s)
{
	s.erase(s.begin(), std::ranges::find_if(s, [](const unsigned char ch) {
		return !std::isspace(ch);
	}));
	s.erase(std::find_if(s.rbegin(), s.rend(), [](const unsigned char ch) {
		return !std::isspace(ch);
	}).base(),
		s.end());
	return s;
}
