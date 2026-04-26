#include "CodeGenerator//CodeGenerator.h"
#include "Frontend/Lexer/Lexer.h"
#include "Frontend/Parser/Parser.h"
#include "Frontend/Parser/SLRTableBuilder.h"
#include "VM/VirtualMachine.h"

#include <fstream>

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
		std::cout << "TERM: " << node->token->value << " (" << TOKEN_NAMES.at(node->token->type) << ")" << std::endl;
	}
	else
	{
		std::cout << "NODE: ID[" << node->symbolId << "], TYPE[" << static_cast<int>(node->type) << "]" << std::endl;
	}

	for (auto& child : node->children)
	{
		PrintAST(child, indent + 1, builder);
	}
}

std::string ReadFile(const std::string& path)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open file: " + path);
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

Grammar SetupGrammar()
{
	Grammar g;
	g.AddRule(NT("Program"), { NT("StatementList") }, true, AstNodeType::PROGRAM);
	g.AddRule(NT("StatementList"), { NT("Statement"), T("SEMICOLON"), NT("StatementList") });
	g.AddRule(NT("StatementList"), { NT("Statement"), T("SEMICOLON") });
	g.AddRule(NT("Statement"), { T("VAR"), T("IDENTIFIER"), T("OP_ASSIGN"), NT("Expression") }, false, AstNodeType::VAR_DECL);
	g.AddRule(NT("Expression"), { T("INT") }, false, AstNodeType::LITERAL);

	return g;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " <source_file.my_lang>" << std::endl;
		return 1;
	}

	std::string filePath = argv[1];

	try
	{
		std::string sourceCode = ReadFile(filePath);
		std::cout << "[1/5] File read successfully." << std::endl;

		const auto myGrammar = SetupGrammar();
		SLRTableBuilder slrBuilder(myGrammar);
		std::cout << "[2/5] Grammar and SLR table built." << std::endl;

		Lexer lexer(sourceCode);
		Parser parser(slrBuilder, lexer);
		auto astRoot = parser.Parse();
		std::cout << "[3/5] Parsing finished. AST created." << std::endl;

		PrintAST(astRoot, 0, slrBuilder);

		CodeGenerator generator;
		auto chunk = generator.Generate(astRoot);
		std::cout << "[4/5] Bytecode generated. Constants: " << chunk->constants.size() << std::endl;

		std::cout << "[5/5] Starting Virtual Machine...\n"
				  << std::endl;
		VirtualMachine vm;
		vm.Interpret(std::move(chunk));

		std::cout << "\nExecution finished successfully." << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "\n[!] COMPILATION ERROR:\n"
				  << e.what() << std::endl;
		return 1;
	}

	return 0;
}
