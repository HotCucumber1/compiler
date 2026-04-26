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

	bool needsAugment = false;

	auto it = rulesMap.find(axiom);
	if (it != rulesMap.end() && it->second.size() > 1)
	{
		needsAugment = true;
	}
	else
	{
		for (const auto& rhsList : rulesMap | std::views::values)
		{
			for (const auto& rhs : rhsList)
			{
				for (const auto& sym : rhs)
				{
					if (sym == axiom)
					{
						needsAugment = true;
						break;
					}
				}
			}
		}
	}

	if (needsAugment)
	{
		RuleItem newAxiom = { axiom.ruleName + "'", false };
		m_flatRules.push_back({ newAxiom, { axiom } });
	}

	for (const auto& [lhs, rhsList] : rulesMap)
	{
		for (const auto& rhs : rhsList)
		{
			std::vector<RuleItem> cleanRhs;
			for (const auto& sym : rhs)
			{
				if (sym.ruleName != Grammar::EPSILON)
				{
					cleanRhs.push_back(sym);
					m_allSymbols.insert(sym);
				}
			}
			m_allSymbols.insert(lhs);
			m_flatRules.emplace_back(lhs, cleanRhs);
		}
	}

	m_allSymbols.insert(RuleItem{ Grammar::END_OF_FILE, true });

	const auto targetAxiom = needsAugment
		? RuleItem{ axiom.ruleName + "'", false }
		: axiom;
	for (size_t i = 0; i < m_flatRules.size(); ++i)
	{
		if (m_flatRules[i].first == targetAxiom)
		{
			m_startRuleIdx = i;
			break;
		}
	}
}

void SLRTableBuilder::ComputeFirstSets()
{
	bool changed = true;
	while (changed)
	{
		changed = false;
		for (const auto& rule : m_flatRules)
		{
			const auto& lhs = rule.first;
			const auto& rhs = rule.second;
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
	m_followSets[m_flatRules[m_startRuleIdx].first].insert(RuleItem{ Grammar::END_OF_FILE, true });
	bool changed = true;
	while (changed)
	{
		changed = false;
		for (const auto& rule : m_flatRules)
		{
			const auto& lhs = rule.first;
			const auto& rhs = rule.second;

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
			if (item.dotPos < m_flatRules[item.ruleIdx].second.size())
			{
				RuleItem sym = m_flatRules[item.ruleIdx].second[item.dotPos];
				if (!sym.isTerminal)
				{
					for (size_t i = 0; i < m_flatRules.size(); ++i)
					{
						if (m_flatRules[i].first == sym)
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
		if (item.dotPos < m_flatRules[item.ruleIdx].second.size())
		{
			if (m_flatRules[item.ruleIdx].second[item.dotPos] == symbol)
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

	std::vector<std::set<LR0Item>> states;
	std::map<std::set<LR0Item>, size_t> stateToIndex;
	std::vector<std::string> stateNames; // Хранит названия в формате S_0_1

	std::set<LR0Item> startState = Closure({ { m_startRuleIdx, 0 } });
	states.push_back(startState);
	stateToIndex[startState] = 0;

	// Имя первого стейта тоже генерируем динамически
	stateNames.push_back(m_flatRules[m_startRuleIdx].first.ruleName + "_" + std::to_string(m_startRuleIdx) + "0");

	// BFS для построения состояний
	for (size_t i = 0; i < states.size(); ++i)
	{
		std::string stateStr = stateNames[i];

		// Обработка SHIFT и GOTO
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

				// Формируем имя для нового состояния: "символ_номерПравила_номерСимвола"
				std::string newStateName = sym.ruleName;
				bool isFirstItem = true;

				for (const auto& item : states[i])
				{
					// Ищем все пункты, по которым произошел этот переход
					if (item.dotPos < m_flatRules[item.ruleIdx].second.size() && m_flatRules[item.ruleIdx].second[item.dotPos] == sym)
					{
						if (isFirstItem)
						{
							newStateName += "_";
							isFirstItem = false;
						}
						else
						{
							newStateName += "-"; // Разделитель для склеенных правил
						}

						// Добавляем индекс правила и позицию точки (+1, т.к. мы шагнули через символ)
						newStateName += std::to_string(item.ruleIdx) + std::to_string(item.dotPos + 1);
					}
				}
				stateNames.push_back(newStateName);
			}

			const auto nextIdx = stateToIndex[nextState];
			const auto nextStateName = stateNames[nextIdx];

			m_table[stateStr][sym.ruleName] = nextStateName;
		}

		for (const auto& item : states[i])
		{
			if (item.dotPos != m_flatRules[item.ruleIdx].second.size())
			{
				continue;
			}
			if (item.ruleIdx == m_startRuleIdx)
			{
				m_table[stateStr][Grammar::END_OF_FILE] = "END";
				continue;
			}
			RuleItem lhs = m_flatRules[item.ruleIdx].first;
			std::string reduceStr = "R" + std::to_string(item.ruleIdx);

			for (const auto& followSym : m_followSets[lhs])
			{
				auto& cell = m_table[stateStr][followSym.ruleName];
				if (cell.empty())
				{
					cell = reduceStr;
				}
				else if (cell != reduceStr && cell.find(reduceStr) == std::string::npos)
				{
					cell += "/" + reduceStr;
				}
			}
		}
	}
	m_orderedStates = stateNames;
}

void SLRTableBuilder::Print(std::ostream& out) const
{
	out << "--- Grammar rules ---" << std::endl;
	for (size_t i = 0; i < m_flatRules.size(); ++i)
	{
		out << i << ": " << m_flatRules[i].first.ruleName << " -> ";
		if (m_flatRules[i].second.empty())
		{
			out << "eps";
		}
		for (const auto& sym : m_flatRules[i].second)
		{
			out << sym.ruleName << " ";
		}
		out << std::endl;
	}
	out << "\n----- SLR Table -----\n";

	std::vector<std::string> terminals;
	std::vector<std::string> nonTerminals;

	for (const auto& sym : m_allSymbols)
	{
		if (sym.isTerminal)
		{
			terminals.push_back(sym.ruleName);
			continue;
		}
		nonTerminals.push_back(sym.ruleName);
	}
	constexpr int colWidth = 10;

	out << std::left << std::setw(colWidth) << "State";
	for (const auto& nt : nonTerminals)
	{
		out << std::setw(colWidth) << nt;
	}
	for (const auto& t : terminals)
	{
		out << std::setw(colWidth) << t;
	}
	out << "\n";

	for (const auto& stateStr : m_orderedStates)
	{
		if (!m_table.contains(stateStr))
		{
			continue;
		}
		out << std::left << std::setw(colWidth) << stateStr;

		const auto& rowMap = m_table.at(stateStr);

		for (const auto& nt : nonTerminals)
		{
			auto it = rowMap.find(nt);
			out << std::setw(colWidth) << (it != rowMap.end() ? it->second : "");
		}
		for (const auto& t : terminals)
		{
			auto it = rowMap.find(t);
			out << std::setw(colWidth) << (it != rowMap.end() ? it->second : "");
		}
		out << std::endl;
	}
}