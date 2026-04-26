#pragma once
#include "Grammar.h"

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

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
	using Table = std::unordered_map<
		std::string, std::unordered_map<std::string, std::string>>;

public:
	explicit SLRTableBuilder(Grammar grammar);
	void Print(std::ostream& out = std::cout) const;

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

	size_t m_startRuleIdx = 0;
};