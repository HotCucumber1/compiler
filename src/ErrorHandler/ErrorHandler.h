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

class CompilerError final : public std::runtime_error
{
public:
	SourceLocation location;

	CompilerError(SourceLocation loc, const std::string& message)
		: std::runtime_error(message)
		, location(std::move(loc))
	{
	}
};
