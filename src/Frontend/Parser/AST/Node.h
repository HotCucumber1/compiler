#pragma once
#include "../../Common/Token.h"

#include <memory>
#include <optional>
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
	SymbolId symbolId;
	std::optional<Token> token;
	std::vector<std::shared_ptr<AstNode>> children;

	explicit AstNode(const SymbolId id)
		: symbolId(id)
	{
	}

	AstNode(const SymbolId id, Token t)
		: symbolId(id)
		, token(std::move(t))
	{
	}
};
