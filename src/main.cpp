#include "CodeGenerator//CodeGenerator.h"
#include "Frontend/Lexer/Lexer.h"
#include "Frontend/Parser/Common/AstPrinter.h"
#include "Frontend/Parser/Grammar/GrammarReader.h"
#include "Frontend/Parser/Parser.h"
#include "Frontend/Parser/SLRTableBuilder.h"
#include "VM/VirtualMachine.h"

#include <filesystem>
#include <fstream>

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

Grammar SetupGrammar(const std::string& exePath)
{
	const auto binaryPath = std::filesystem::absolute(exePath);
	const auto binaryDir = binaryPath.parent_path();
	const auto grammarPath = binaryDir / "../grammar";

	const auto grammar = GrammarReader::Read(grammarPath);

	// grammar.Print();

	return grammar;
}

void PrintConsts(const std::unique_ptr<Chunk>& chunk)
{
	for (const auto& val : chunk->constants)
	{
		PrintValue(val);
	}
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

		const auto myGrammar = SetupGrammar(argv[0]);
		SLRTableBuilder slrBuilder(myGrammar);
		std::cout << "[2/5] Grammar and SLR table built." << std::endl;

		Lexer lexer(sourceCode);
		Parser parser(slrBuilder, lexer);
		auto astRoot = parser.Parse();
		std::cout << "[3/5] Parsing finished. AST created." << std::endl;

		AstPrinter printer;
		printer.PrintAst(astRoot);

		CodeGenerator generator;
		auto chunk = generator.Generate(astRoot);
		std::cout << "[4/5] Bytecode generated. Constants: " << chunk->constants.size() << std::endl;

		// PrintConsts(chunk);

		std::cout << "[5/5] Starting Virtual Machine...\n"
				  << std::endl;
		VirtualMachine vm;
		vm.Interpret(chunk);

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
