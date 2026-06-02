#pragma once
#include "../Common/Token.h"
#include "Reader.h"

#include <string>

class Lexer
{
public:
	explicit Lexer(const std::string& str, std::string filePath = "unknown");

	explicit Lexer(const std::istream& input, std::string filePath = "unknown");

	Token GetToken();

	bool IsEmpty();

private:
	Token Identifier();

	Token Number();

	Token String();

	Token SpecialChar();

private:
	Reader m_reader;
	std::string m_filePath;
};
