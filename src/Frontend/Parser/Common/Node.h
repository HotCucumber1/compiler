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
	UNARY_OP,
	LITERAL,
	TERMINAL,
	CALL,
	VAR_ACCESS,
};

static const std::unordered_map<std::string, AstNodeType> AST_NODE_TYPES = {
	{ "UNDEFINED", AstNodeType::UNDEFINED },
	{ "PROGRAM", AstNodeType::PROGRAM },
	{ "BLOCK", AstNodeType::BLOCK },
	{ "VAR_DECL", AstNodeType::VAR_DECL },
	{ "IF_STATEMENT", AstNodeType::IF_STATEMENT },
	{ "ASSIGNMENT", AstNodeType::ASSIGNMENT },
	{ "EXPRESSION", AstNodeType::EXPRESSION },
	{ "BINARY_OP", AstNodeType::BINARY_OP },
	{ "UNARY_OP", AstNodeType::UNARY_OP },
	{ "LITERAL", AstNodeType::LITERAL },
	{ "TERMINAL", AstNodeType::TERMINAL },
	{ "CALL", AstNodeType::CALL },
	{ "VAR_ACCESS", AstNodeType::VAR_ACCESS },
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
