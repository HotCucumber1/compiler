#pragma once
#include "AST/Node.h"

#include <iostream>
#include <map>
#include <string>
#include <vector>

struct RuleItem
{
	std::string ruleName;
	bool isTerminal;

	auto operator<=>(const RuleItem&) const = default;
};

class Grammar
{
public:
	static constexpr std::string EPSILON = "#";
	static constexpr std::string END_OF_FILE = "$";

	using RulesMap = std::map<RuleItem, std::vector<std::vector<RuleItem>>>;

	const RulesMap& GetRules() const;

	void SetRules(const RulesMap& rules);

	void AddRule(const RuleItem& left, const std::vector<RuleItem>& right, bool isAxiom = false);

	static bool IsNonTerminal(const RuleItem& rule);

	std::vector<RuleItem> GetNonTerminals() const;

	RuleItem GetAxiom() const;

	void Print(std::ostream& out = std::cout) const;

private:
	RulesMap m_rules;
	std::vector<RuleItem> m_orderedNonTerminals;
	RuleItem m_axiom = { "S", false };
};

inline auto NT = [](const std::string& name) { return RuleItem{ name, false }; };
inline auto T = [](const std::string& name) { return RuleItem{ name, true }; };
inline auto EPS = RuleItem{ "#", true };
