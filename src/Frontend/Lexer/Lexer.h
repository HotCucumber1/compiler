#pragma once
#include "../Common/Token.h"
#include "Reader.h"

#include <string>

class Lexer
{
public:
	explicit Lexer(const std::string& str);

	explicit Lexer(const std::istream& input);

	Token GetToken();

	bool IsEmpty();

private:
	Token Identifier();

	Token Number();

	Token String();

	Token SpecialChar();

	Reader m_reader;
};
