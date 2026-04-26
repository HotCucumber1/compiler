#pragma once
#include "../../Common/Token.h"

#include <memory>
#include <tuple>
#include <vector>

using SymbolId = int32_t;

enum class SymbolType
{
	Terminal,
	NonTerminal,
};

struct Symbol
{
	SymbolId id;
	SymbolType type;

	bool operator<(const Symbol& other) const
	{
		return std::tie(id, type) < std::tie(other.id, other.type);
	}
};

struct AstNode
{
	TokenType type;
	std::string value;
	std::vector<std::unique_ptr<AstNode>> children;

	explicit AstNode(
		const TokenType type,
		const std::string& value = "")
		: type(type)
		, value(value)
	{
	}
};
