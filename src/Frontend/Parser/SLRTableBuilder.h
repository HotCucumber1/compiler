#pragma once
#include "AST/Node.h"
#include "Grammar.h"

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

struct TypedRule
{
	SymbolId lhsId;
	SymbolId rhsSize;
	// TokenType resultType;
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

struct LR0Item
{
	size_t ruleIdx;
	size_t dotPos;

	bool operator<(const LR0Item& other) const
	{
		if (ruleIdx != other.ruleIdx)
		{
			return ruleIdx < other.ruleIdx;
		}
		return dotPos < other.dotPos;
	}

	bool operator==(const LR0Item& other) const
	{
		return ruleIdx == other.ruleIdx && dotPos == other.dotPos;
	}
};

class SLRTableBuilder
{
	using Table = std::vector<std::unordered_map<SymbolId, Action>>;

public:
	explicit SLRTableBuilder(Grammar grammar);

	void Print(std::ostream& out = std::cout) const;

	Table GetTable() const
	{
		return m_table;
	}

	std::vector<TypedRule> GetRules() const
	{
		return m_rules;
	}

	SymbolId GetSymbolId(const RuleItem& item) const
	{
		const auto it = m_symbolToId.find(item);
		return it != m_symbolToId.end()
			? it->second
			: -1;
	}

private:
	void BuildTable();

	void PrepareGrammar();

	void ComputeFirstSets();

	void ComputeFollowSets();

	std::set<LR0Item> Closure(const std::set<LR0Item>& items) const;
	std::set<LR0Item> Goto(const std::set<LR0Item>& items, const RuleItem& symbol) const;

private:
	Grammar m_grammar;
	Table m_table;

	std::vector<std::pair<RuleItem, std::vector<RuleItem>>> m_flatRules;
	std::map<RuleItem, std::set<RuleItem>> m_firstSets;
	std::map<RuleItem, std::set<RuleItem>> m_followSets;
	std::set<RuleItem> m_allSymbols;

	std::vector<std::string> m_orderedStates;
	std::vector<TypedRule> m_rules;
	std::map<RuleItem, SymbolId> m_symbolToId;
	std::map<SymbolId, RuleItem> m_idToSymbol;
	SymbolId m_endOfFileId = -1;

	size_t m_startRuleIdx = 0;
};