#pragma once
#include <stdexcept>
#include <string>
#include <utility>

struct SourceLocation
{
	std::string file = "unknown";
	int line = 0;
	int col = 0;
};

class CompilationError : public std::runtime_error
{
public:
	SourceLocation location;

	CompilationError(SourceLocation loc, const std::string& message)
		: std::runtime_error(message)
		, location(std::move(loc))
	{
	}
};

class SemanticError final : public CompilationError
{
public:
	SemanticError(const SourceLocation& loc, const std::string& message)
		: CompilationError(loc, "Semantic error: " + message)
	{
	}
};

class SyntaxError final : public CompilationError
{
public:
	SyntaxError(const SourceLocation& loc, const std::string& message)
		: CompilationError(loc, "Syntax error: " + message)
	{
	}
};
