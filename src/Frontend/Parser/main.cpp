#include "../Lexer/Lexer.h"
#include "Common/Grammar.h"
#include "Parser.h"
#include "SLRTableBuilder.h"
#include <iostream>

void PrintAST(
	const std::shared_ptr<AstNode>& node,
	int indent,
	const SLRTableBuilder& builder);

int main()
{
	try
	{
		Grammar myLang;
		myLang.AddRule(NT("Program"), { NT("StatementList") }, true);
		myLang.AddRule(NT("StatementList"), { NT("Statement"), T("SEMICOLON"), NT("StatementList") });
		myLang.AddRule(NT("StatementList"), { NT("Statement"), T("SEMICOLON") });

		myLang.AddRule(NT("Statement"), { T("IDENTIFIER"), T("OP_ASSIGN"), T("INT") });

		const SLRTableBuilder builder(myLang);

		const std::string code = "x = 42; y = 100;";
		Lexer lexer(code);

		const Parser parser(builder, lexer);
		const auto astRoot = parser.Parse();

		std::cout << "Success! Tree built." << std::endl;

		PrintAST(astRoot, 0, builder);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Compiler Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}

void PrintAST(const std::shared_ptr<AstNode>& node, int indent, const SLRTableBuilder& builder)
{
	if (!node)
		return;

	for (int i = 0; i < indent; ++i)
	{
		std::cout << "  ";
	}

	if (node->token.has_value())
	{
		// Лист дерева (терминал)
		std::cout << "TERM: " << node->token->value << " (" << TOKEN_NAMES.at(node->token->type) << ")" << std::endl;
	}
	else
	{
		// Узел дерева (нетерминал)
		// Тут можно вытащить имя нетерминала через builder.GetSymbolName(node->symbolId)
		std::cout << "NODE: ID[" << node->symbolId << "]" << std::endl;
	}

	for (auto& child : node->children)
	{
		PrintAST(child, indent + 1, builder);
	}
}