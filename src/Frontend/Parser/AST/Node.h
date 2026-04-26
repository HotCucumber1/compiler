#pragma once
#include "../../Common/Token.h"

#include <cstdint>
#include <memory>
#include <tuple>
#include <unordered_map>
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

struct TypedRule
{
	SymbolId lhsId;
	SymbolId rhsSize;
	TokenType resultType;
};

enum class ActionType
{
	Error,
	Shift,
	Reduce,
	Accept,
};

struct Action
{
	ActionType type = ActionType::Error;
	int32_t value = -1;

	static Action Shift(const int state)
	{
		return { ActionType::Shift, state };
	}

	static Action Reduce(const int rule)
	{
		return { ActionType::Reduce, rule };
	}

	static Action Accept()
	{
		return { ActionType::Accept, 0 };
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
