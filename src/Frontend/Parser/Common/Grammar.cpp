#include "Grammar.h"

const Grammar::RulesMap& Grammar::GetRules() const
{
	return m_rules;
}

void Grammar::SetRules(const RulesMap& rules)
{
	m_rules = rules;
}

void Grammar::AddRule(
	const RuleItem& left,
	const std::vector<RuleItem>& right,
	const bool isAxiom,
	const AstNodeType nodeType)
{
	if (!m_rules.contains(left))
	{
		m_orderedNonTerminals.push_back(left);
	}

	m_rules[left].push_back({ right, nodeType });
	if (isAxiom)
	{
		m_axiom = left;
	}
}

bool Grammar::IsNonTerminal(const RuleItem& rule)
{
	return !rule.isTerminal;
}

std::vector<RuleItem> Grammar::GetNonTerminals() const
{
	return m_orderedNonTerminals;
}

RuleItem Grammar::GetAxiom() const
{
	return m_axiom;
}

void Grammar::Print(std::ostream& out) const
{
	for (const auto& [left, definitions] : m_rules)
	{
		for (const auto& def : definitions)
		{
			out << left.ruleName << " -> ";

			if (def.rhs.empty())
			{
				out << "ε";
			}
			else
			{
				for (const auto& item : def.rhs)
				{
					out << item.ruleName << " ";
				}
			}
			if (def.nodeType != AstNodeType::UNDEFINED)
			{
				out << " [Node: " << static_cast<int>(def.nodeType) << "]";
			}

			out << std::endl;
		}
	}
}
