#pragma once
#include "../../Common/Token.h"

#include <memory>
#include <optional>
#include <vector>

enum class AstNodeType : int
{
	UNDEFINED,
	PROGRAM,
	BLOCK,
	VAR_DECL,
	IF_STATEMENT,
	ASSIGNMENT,
	EXPRESSION,
	BINARY_OP,
	LITERAL,
	TERMINAL,
};

using SymbolId = int32_t;

struct AstNode
{
	SymbolId symbolId;
	std::optional<Token> token;
	AstNodeType type = AstNodeType::UNDEFINED;
	std::vector<std::shared_ptr<AstNode>> children;

	explicit AstNode(const SymbolId id, const AstNodeType type)
		: symbolId(id)
		, type(type)
	{
	}

	AstNode(const SymbolId id, Token t, const AstNodeType type)
		: symbolId(id)
		, token(std::move(t))
		, type(type)
	{
	}
};
