#pragma once
#include "../../Common/Token.h"

#include <algorithm>

inline std::string ToLower(std::string str)
{
	std::ranges::transform(str, str.begin(), [](const unsigned char c) {
		return std::tolower(c);
	});
	return str;
}

inline Token CheckKeywords(const Token& token)
{
	const auto value = ToLower(token.value);
	const auto it = KEYWORDS.find(value);
	return it == KEYWORDS.end()
		? token
		: Token{
			  token.pos,
			  token.line,
			  it->second,
			  value,
		  };
}