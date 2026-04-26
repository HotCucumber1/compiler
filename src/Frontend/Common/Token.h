#pragma once
#include <string>
#include <unordered_map>

enum class TokenType
{
	// Base
	IDENTIFIER,
	INT,
	FLOAT,
	STRING,
	TRUE,
	FALSE,
	CONST,

	VAR,
	MAIN,
	END,
	BEGIN,

	// Base types
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_STRING,
	TYPE_BOOL,

	// Delimiters
	SEMICOLON,
	COMMA,
	COLON,

	// Brackets
	BRACKET_BASE_OPEN,
	BRACKET_BASE_CLOSE,
	BRACKET_CURLY_OPEN,
	BRACKET_CURLY_CLOSE,
	BRACKET_SQUARE_OPEN,
	BRACKET_SQUARE_CLOSE,

	// Keywords
	IF,
	ELSE,
	WHILE,
	FOR,
	CONTINUE,
	BREAK,
	NOT,
	AND,
	OR,
	FUNCTION,
	RETURN,
	PUBLIC,
	PRIVATE,
	CLASS,
	IMPLEMENTS,
	EXTENDS,
	THROW,
	TRY,
	CATCH,
	NEW,
	FINALLY,

	// Math
	OP_PLUS,
	OP_MINUS,
	OP_MULTI,
	OP_DIVISION,
	OP_DIV,
	OP_MOD,
	OP_ASSIGN,
	OP_INCREMENT,
	OP_DECREMENT,

	// Comparisons
	OP_LESS,
	OP_MORE,
	OP_EQUAL,
	OP_NOT_EQUAL,
	OP_LESS_OR_EQUAL,
	OP_MORE_OR_EQUAL,

	// Another ops
	OP_TERN_THEN,
	OP_TERN_ELSE,
	OP_DOT,

	ERROR,
};

struct Token
{
	int pos;
	int line;
	TokenType type;
	std::string value;
};

static const std::unordered_map<std::string, TokenType> KEYWORDS = {
	{ "if", TokenType::IF },
	{ "else", TokenType::ELSE },
	{ "while", TokenType::WHILE },
	{ "for", TokenType::FOR },
	{ "continue", TokenType::CONTINUE },
	{ "break", TokenType::BREAK },
	{ "not", TokenType::NOT },
	{ "and", TokenType::AND },
	{ "or", TokenType::OR },
	{ "function", TokenType::FUNCTION },
	{ "return", TokenType::RETURN },
	{ "public", TokenType::PUBLIC },
	{ "private", TokenType::PRIVATE },
	{ "class", TokenType::CLASS },
	{ "implements", TokenType::IMPLEMENTS },
	{ "extends", TokenType::EXTENDS },
	{ "throw", TokenType::THROW },
	{ "try", TokenType::TRY },
	{ "catch", TokenType::CATCH },
	{ "new", TokenType::NEW },
	{ "finally", TokenType::FINALLY },

	{ "int", TokenType::TYPE_INT },
	{ "float", TokenType::TYPE_FLOAT },
	{ "string", TokenType::TYPE_STRING },
	{ "true", TokenType::TRUE },
	{ "false", TokenType::FALSE },
	{ "const", TokenType::CONST },
	{ "bool", TokenType::TYPE_BOOL },

	{ "var", TokenType::VAR },
	{ "main", TokenType::MAIN },
	{ "end", TokenType::END },
	{ "begin", TokenType::BEGIN },
};

const std::unordered_map<char, TokenType> SPECIAL_CHARS = {
	{ '(', TokenType::BRACKET_BASE_OPEN },
	{ ')', TokenType::BRACKET_BASE_CLOSE },
	{ '{', TokenType::BRACKET_CURLY_OPEN },
	{ '}', TokenType::BRACKET_CURLY_CLOSE },
	{ '[', TokenType::BRACKET_SQUARE_OPEN },
	{ ']', TokenType::BRACKET_SQUARE_CLOSE },
	{ ',', TokenType::COMMA },
	{ '+', TokenType::OP_PLUS },
	{ '-', TokenType::OP_MINUS },
	{ '*', TokenType::OP_MULTI },
	{ '/', TokenType::OP_DIVISION },
	{ '=', TokenType::OP_ASSIGN },
	{ '<', TokenType::OP_LESS },
	{ '>', TokenType::OP_MORE },
	{ ';', TokenType::SEMICOLON },
	{ ':', TokenType::COLON },
	{ '.', TokenType::OP_DOT },
};

const std::unordered_map<char, std::pair<char, TokenType>> SPECIAL_CHARS_DOUBLE = {
	{ '=', { '=', TokenType::OP_EQUAL } },
	{ '!', { '=', TokenType::OP_NOT_EQUAL } },
	{ '<', { '=', TokenType::OP_LESS_OR_EQUAL } },
	{ '>', { '=', TokenType::OP_MORE_OR_EQUAL } },
	{ '+', { '+', TokenType::OP_INCREMENT } },
	{ '-', { '-', TokenType::OP_DECREMENT } },
};
const std::unordered_map<TokenType, std::string> TOKEN_NAMES = {
	{ TokenType::IDENTIFIER, "IDENTIFIER" },
	{ TokenType::INT, "INT" },
	{ TokenType::FLOAT, "FLOAT" },
	{ TokenType::STRING, "STRING" },
	{ TokenType::TRUE, "TRUE" },
	{ TokenType::FALSE, "FALSE" },
	{ TokenType::CONST, "CONST" },

	// Base types
	{ TokenType::TYPE_INT, "TYPE_INT" },
	{ TokenType::TYPE_FLOAT, "TYPE_FLOAT" },
	{ TokenType::TYPE_STRING, "TYPE_STRING" },
	{ TokenType::TYPE_BOOL, "TYPE_BOOL" },

	// Delimiters
	{ TokenType::SEMICOLON, "SEMICOLON" },
	{ TokenType::COMMA, "COMMA" },

	// Brackets
	{ TokenType::BRACKET_BASE_OPEN, "BRACKET_BASE_OPEN" },
	{ TokenType::BRACKET_BASE_CLOSE, "BRACKET_BASE_CLOSE" },
	{ TokenType::BRACKET_CURLY_OPEN, "BRACKET_CURLY_OPEN" },
	{ TokenType::BRACKET_CURLY_CLOSE, "BRACKET_CURLY_CLOSE" },
	{ TokenType::BRACKET_SQUARE_OPEN, "BRACKET_SQUARE_OPEN" },
	{ TokenType::BRACKET_SQUARE_CLOSE, "BRACKET_SQUARE_CLOSE" },

	// Keywords
	{ TokenType::IF, "IF" },
	{ TokenType::ELSE, "ELSE" },
	{ TokenType::WHILE, "WHILE" },
	{ TokenType::FOR, "FOR" },
	{ TokenType::CONTINUE, "CONTINUE" },
	{ TokenType::BREAK, "BREAK" },
	{ TokenType::NOT, "NOT" },
	{ TokenType::AND, "AND" },
	{ TokenType::OR, "OR" },
	{ TokenType::FUNCTION, "FUNCTION" },
	{ TokenType::RETURN, "RETURN" },
	{ TokenType::PUBLIC, "PUBLIC" },
	{ TokenType::PRIVATE, "PRIVATE" },
	{ TokenType::CLASS, "CLASS" },
	{ TokenType::IMPLEMENTS, "IMPLEMENTS" },
	{ TokenType::EXTENDS, "EXTENDS" },
	{ TokenType::THROW, "THROW" },
	{ TokenType::TRY, "TRY" },
	{ TokenType::CATCH, "CATCH" },
	{ TokenType::NEW, "NEW" },
	{ TokenType::FINALLY, "FINALLY" },

	// Math
	{ TokenType::OP_PLUS, "OP_PLUS" },
	{ TokenType::OP_MINUS, "OP_MINUS" },
	{ TokenType::OP_MULTI, "OP_MULTI" },
	{ TokenType::OP_DIVISION, "OP_DIVISION" },
	{ TokenType::OP_DIV, "OP_DIV" },
	{ TokenType::OP_MOD, "OP_MOD" },
	{ TokenType::OP_ASSIGN, "OP_ASSIGN" },
	{ TokenType::OP_INCREMENT, "OP_INCREMENT" },
	{ TokenType::OP_DECREMENT, "OP_DECREMENT" },

	// Comparisons
	{ TokenType::OP_LESS, "OP_LESS" },
	{ TokenType::OP_MORE, "OP_MORE" },
	{ TokenType::OP_EQUAL, "OP_EQUAL" },
	{ TokenType::OP_NOT_EQUAL, "OP_NOT_EQUAL" },
	{ TokenType::OP_LESS_OR_EQUAL, "OP_LESS_OR_EQUAL" },
	{ TokenType::OP_MORE_OR_EQUAL, "OP_MORE_OR_EQUAL" },

	// Another ops
	{ TokenType::OP_TERN_THEN, "OP_TERN_THEN" },
	{ TokenType::OP_TERN_ELSE, "OP_TERN_ELSE" },
	{ TokenType::OP_DOT, "OP_DOT" },

	// Error
	{ TokenType::ERROR, "ERROR" },

	{ TokenType::VAR, "VAR" },
	{ TokenType::MAIN, "MAIN" },
	{ TokenType::END, "END" },
	{ TokenType::BEGIN, "BEGIN" },
	{ TokenType::COLON, ":" },
};
