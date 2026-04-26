#include "SLRTableBuilder.h"

Grammar GetSimpleGrammar();
Grammar GetWithLeftRecursion();
Grammar GetJustAnotherGrammar();
Grammar GetGrammarWithEpsilon();
Grammar GetZeroOneGrammar();

int main()
{
	try
	{
		const auto gr = GetZeroOneGrammar();
		const SLRTableBuilder table(gr);

		table.Print();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
}

Grammar GetZeroOneGrammar()
{
	Grammar grammar;

	grammar.AddRule(NT("S"), { T("0"), NT("S"), T("1") });
	grammar.AddRule(NT("S"), { T("0"), T("1") });

	return grammar;
}

Grammar GetSimpleGrammar()
{
	Grammar grammar;

	grammar.AddRule(NT("S"), { T("a"), NT("S"), T("b") }, true);
	grammar.AddRule(NT("S"), { T("c") }, true);

	return grammar;
}

Grammar GetWithLeftRecursion()
{
	Grammar grammar;

	grammar.AddRule(NT("S"), { NT("A"), NT("B") }, true);
	grammar.AddRule(NT("A"), { T("a") });
	grammar.AddRule(NT("A"), { NT("A"), T("c") });
	grammar.AddRule(NT("B"), { T("b"), NT("A") });

	return grammar;
}

Grammar GetJustAnotherGrammar()
{
	Grammar grammar;

	grammar.AddRule(NT("S"), { T("a"), NT("A"), T("b") }, true);
	grammar.AddRule(NT("S"), { T("c"), NT("B"), T("b") }, true);

	grammar.AddRule(NT("A"), { T("x"), NT("A"), T("y") });
	grammar.AddRule(NT("A"), { T("a") });

	grammar.AddRule(NT("B"), { T("y"), NT("B"), T("x") });
	grammar.AddRule(NT("B"), { T("b") });

	return grammar;
}

Grammar GetGrammarWithEpsilon()
{
	Grammar grammar;

	grammar.AddRule(NT("S"), { NT("A"), NT("B"), NT("C") }, true);

	grammar.AddRule(NT("A"), { NT("A"), T("a") });
	grammar.AddRule(NT("A"), { EPS });

	grammar.AddRule(NT("B"), { T("b"), NT("B") });
	grammar.AddRule(NT("B"), { EPS });

	grammar.AddRule(NT("C"), { NT("C"), T("c") });
	grammar.AddRule(NT("C"), { EPS });

	return grammar;
}