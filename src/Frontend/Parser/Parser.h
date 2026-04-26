#pragma once
#include "../Lexer/Lexer.h"
#include "SLRTableBuilder.h"

class Parser
{
public:
	Parser(const SLRTableBuilder& tableBuilder, Lexer& lexer);

	std::shared_ptr<AstNode> Parse() const;

private:
	SymbolId GetSymbolId(const Token& token) const;

private:
	const std::vector<std::unordered_map<SymbolId, Action>> m_table;
	const std::vector<TypedRule> m_rules;
	const SLRTableBuilder m_builder;
	Lexer& m_lexer;
};
