#include "SLRTableBuilder.h"

#include <iomanip>
#include <ranges>

SLRTableBuilder::SLRTableBuilder(Grammar grammar)
	: m_grammar(std::move(grammar))
{
	BuildTable();
}

void SLRTableBuilder::PrepareGrammar()
{
	RuleItem axiom = m_grammar.GetAxiom();
	const auto& rulesMap = m_grammar.GetRules();

	m_flatRules.clear();
	m_rules.clear();
	m_allSymbols.clear();

	RuleItem newAxiom = { axiom.ruleName + "'", false };
	m_flatRules.push_back({ newAxiom, { axiom }, AstNodeType::PROGRAM });

	for (const auto& [lhs, definitions] : rulesMap)
	{
		for (const auto& def : definitions)
		{
			std::vector<RuleItem> cleanRhs;
			for (const auto& sym : def.rhs)
			{
				if (sym.ruleName != Grammar::EPSILON)
				{
					cleanRhs.push_back(sym);
					m_allSymbols.insert(sym);
				}
			}
			m_allSymbols.insert(lhs);

			m_flatRules.push_back({ lhs, cleanRhs, def.nodeType });
		}
	}

	const RuleItem eofItem{ Grammar::END_OF_FILE, true };
	m_allSymbols.insert(eofItem);

	SymbolId currentId = 0;
	for (const auto& sym : m_allSymbols)
	{
		m_symbolToId[sym] = currentId;
		m_idToSymbol[currentId] = sym;
		if (sym == eofItem)
		{
			m_endOfFileId = currentId;
		}
		currentId++;
	}

	for (const auto& flatRule : m_flatRules)
	{
		m_rules.push_back({ m_symbolToId[flatRule.lhs],
			static_cast<SymbolId>(flatRule.rhs.size()),
			flatRule.type });
	}

	m_startRuleIdx = 0;
}

void SLRTableBuilder::ComputeFirstSets()
{
	bool changed = true;
	while (changed)
	{
		changed = false;
		for (const auto& rule : m_flatRules)
		{
			const auto& lhs = rule.lhs;
			const auto& rhs = rule.rhs;
			const auto beforeSize = m_firstSets[lhs].size();

			bool allCanBeEmpty = true;
			for (const auto& sym : rhs)
			{
				if (sym.isTerminal)
				{
					m_firstSets[lhs].insert(sym);
					allCanBeEmpty = false;
					break;
				}
				bool hasEps = false;
				for (const auto& firstSym : m_firstSets[sym])
				{
					if (firstSym.ruleName == Grammar::EPSILON)
					{
						hasEps = true;
					}
					else
					{
						m_firstSets[lhs].insert(firstSym);
					}
				}
				if (!hasEps)
				{
					allCanBeEmpty = false;
					break;
				}
			}
			if (allCanBeEmpty)
			{
				m_firstSets[lhs].insert(RuleItem{ Grammar::EPSILON, true });
			}

			if (m_firstSets[lhs].size() > beforeSize)
			{
				changed = true;
			}
		}
	}
}

void SLRTableBuilder::ComputeFollowSets()
{
	m_followSets[m_flatRules[m_startRuleIdx].lhs].insert(RuleItem{ Grammar::END_OF_FILE, true });
	bool changed = true;
	while (changed)
	{
		changed = false;
		for (const auto& rule : m_flatRules)
		{
			const auto& lhs = rule.lhs;
			const auto& rhs = rule.rhs;

			for (size_t i = 0; i < rhs.size(); ++i)
			{
				if (rhs[i].isTerminal)
				{
					continue;
				}

				const auto beforeSize = m_followSets[rhs[i]].size();
				bool allCanBeEmpty = true;

				for (size_t j = i + 1; j < rhs.size(); ++j)
				{
					if (rhs[j].isTerminal)
					{
						m_followSets[rhs[i]].insert(rhs[j]);
						allCanBeEmpty = false;
						break;
					}
					bool hasEps = false;
					for (const auto& firstSym : m_firstSets[rhs[j]])
					{
						if (firstSym.ruleName == Grammar::EPSILON)
						{
							hasEps = true;
						}
						else
						{
							m_followSets[rhs[i]].insert(firstSym);
						}
					}
					if (!hasEps)
					{
						allCanBeEmpty = false;
						break;
					}
				}

				if (allCanBeEmpty)
				{
					for (const auto& followSym : m_followSets[lhs])
					{
						m_followSets[rhs[i]].insert(followSym);
					}
				}

				if (m_followSets[rhs[i]].size() > beforeSize)
				{
					changed = true;
				}
			}
		}
	}
}

std::set<LR0Item> SLRTableBuilder::Closure(const std::set<LR0Item>& items) const
{
	std::set<LR0Item> result = items;
	bool changed = true;

	while (changed)
	{
		changed = false;
		std::set<LR0Item> newItems = result;

		for (const auto& item : result)
		{
			if (item.dotPos < m_flatRules[item.ruleIdx].rhs.size())
			{
				RuleItem sym = m_flatRules[item.ruleIdx].rhs[item.dotPos];
				if (!sym.isTerminal)
				{
					for (size_t i = 0; i < m_flatRules.size(); ++i)
					{
						if (m_flatRules[i].lhs == sym)
						{
							LR0Item newItem{ i, 0 };
							if (newItems.insert(newItem).second)
							{
								changed = true;
							}
						}
					}
				}
			}
		}
		result = newItems;
	}
	return result;
}

std::set<LR0Item> SLRTableBuilder::Goto(const std::set<LR0Item>& items, const RuleItem& symbol) const
{
	std::set<LR0Item> nextItems;
	for (const auto& item : items)
	{
		if (item.dotPos < m_flatRules[item.ruleIdx].rhs.size())
		{
			if (m_flatRules[item.ruleIdx].rhs[item.dotPos] == symbol)
			{
				nextItems.insert({ item.ruleIdx, item.dotPos + 1 });
			}
		}
	}
	return Closure(nextItems);
}

void SLRTableBuilder::BuildTable()
{
	PrepareGrammar();
	ComputeFirstSets();
	ComputeFollowSets();

	m_table.clear();

	std::vector<std::set<LR0Item>> states;
	std::map<std::set<LR0Item>, size_t> stateToIndex;

	const std::set<LR0Item> startState = Closure({ { m_startRuleIdx, 0 } });
	states.push_back(startState);
	stateToIndex[startState] = 0;

	for (size_t i = 0; i < states.size(); ++i)
	{
		if (m_table.size() <= i)
		{
			m_table.emplace_back();
		}

		for (const auto& sym : m_allSymbols)
		{
			auto nextState = Goto(states[i], sym);
			if (nextState.empty())
			{
				continue;
			}
			if (!stateToIndex.contains(nextState))
			{
				stateToIndex[nextState] = states.size();
				states.push_back(nextState);
			}

			const auto nextIdx = stateToIndex[nextState];
			m_table[i][m_symbolToId[sym]] = Action{
				ActionType::Shift,
				static_cast<SymbolId>(nextIdx)
			};
		}

		for (const auto& item : states[i])
		{
			if (item.dotPos != m_flatRules[item.ruleIdx].rhs.size())
			{
				continue;
			}
			if (item.ruleIdx == m_startRuleIdx)
			{
				m_table[i][m_endOfFileId] = Action{ ActionType::Accept, 0 };
				continue;
			}

			const auto lhs = m_flatRules[item.ruleIdx].lhs;

			for (const auto& followSym : m_followSets[lhs])
			{
				SymbolId followId = m_symbolToId[followSym];

				if (m_table[i].contains(followId))
				{
					const auto [type, value] = m_table[i][followId];
					if (type == ActionType::Shift)
					{
						std::cerr << "Conflict Shift/Reduce in state " << i << " on symbol " << followSym.ruleName << std::endl;
					}
				}
				m_table[i][followId] = Action{ ActionType::Reduce, static_cast<int>(item.ruleIdx) };
			}
		}
	}
}

void SLRTableBuilder::Print(std::ostream& out) const
{
	out << "--- Grammar rules ---" << std::endl;
	for (size_t i = 0; i < m_flatRules.size(); ++i)
	{
		out << i << ": " << m_flatRules[i].lhs.ruleName << " -> ";
		if (m_flatRules[i].rhs.empty())
		{
			out << "eps";
		}
		for (const auto& sym : m_flatRules[i].rhs)
		{
			out << sym.ruleName << " ";
		}
		out << std::endl;
	}
	out << "\n----- SLR Table -----\n";

	std::vector<SymbolId> terminals;
	std::vector<SymbolId> nonTerminals;

	for (const auto& [sym, id] : m_symbolToId)
	{
		if (sym.isTerminal)
			terminals.push_back(id);
		else
			nonTerminals.push_back(id);
	}

	constexpr int colWidth = 10;

	out << std::left << std::setw(colWidth) << "State";
	for (const auto& ntId : nonTerminals)
	{
		out << std::setw(colWidth) << m_idToSymbol.at(ntId).ruleName;
	}
	for (const auto& tId : terminals)
	{
		out << std::setw(colWidth) << m_idToSymbol.at(tId).ruleName;
	}
	out << "\n";

	for (size_t stateIdx = 0; stateIdx < m_table.size(); ++stateIdx)
	{
		out << std::left << std::setw(colWidth) << stateIdx;

		const auto& rowMap = m_table[stateIdx];

		auto printCell = [&](SymbolId id) {
			auto it = rowMap.find(id);
			if (it == rowMap.end())
			{
				out << std::setw(colWidth) << "";
			}
			else
			{
				const Action& act = it->second;
				std::string cellStr;
				if (act.type == ActionType::Shift)
					cellStr = "S" + std::to_string(act.value);
				else if (act.type == ActionType::Reduce)
					cellStr = "R" + std::to_string(act.value);
				else if (act.type == ActionType::Accept)
					cellStr = "ACC";

				out << std::setw(colWidth) << cellStr;
			}
		};

		for (const auto& ntId : nonTerminals)
			printCell(ntId);
		for (const auto& tId : terminals)
			printCell(tId);

		out << std::endl;
	}
}