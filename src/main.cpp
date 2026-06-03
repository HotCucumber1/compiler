#include "CodeGenerator/CodeGenerator.h"
#include "Common/AstPrinter.h"
#include "Lexer/Lexer.h"
#include "Parser/Grammar/GrammarReader.h"
#include "Parser/Parser.h"
#include "Parser/SLRTableBuilder.h"
#include "SematicAnalyzer/SemanticAnalyzer.h"
#include "Visualizer/ConsoleSchemaVisualizer.h"
#include "Visualizer/SchemaVisualizer.h"
#include "VM/VirtualMachine.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_set>

std::string ReadFile(const std::string& path);
Grammar SetupGrammar(const std::string& exePath);
void PrintConsts(const std::unique_ptr<Chunk>& chunk);

std::string GetSourceCode(const std::string& filePath);
SLRTableBuilder SetupSLRTableBuilder(const std::string& prog);
std::shared_ptr<AstNode> BuildAST(
	const std::string& sourceCode,
	const std::string& filePath,
	const SLRTableBuilder& slrBuilder);
std::unique_ptr<Chunk> GenerateCode(const std::shared_ptr<AstNode>& astRoot);
void RunVM(const std::unique_ptr<Chunk>& chunk);

void ResolveImports(
	const std::shared_ptr<AstNode>& astRoot,
	std::unordered_set<std::string>& parsedFiles,
	const SLRTableBuilder& slrBuilder,
	const std::filesystem::path& currentDirPath);

struct Params
{
	bool printSchema = false;
	bool dumpSchema = false;
	std::string dumpFilename;
	std::string sourceFilePath;
};

Params ParseParams(int argc, char* argv[]);

int main(const int argc, char* argv[])
{
	try
	{
		const auto [printSchema, dumpSchema, dumpFilename, sourceFilePath] = ParseParams(argc, argv);

		const auto mainFilePath = std::filesystem::absolute(sourceFilePath);
		const auto sourceCode = GetSourceCode(sourceFilePath);
		const auto slrBuilder = SetupSLRTableBuilder(argv[0]);
		const auto astRoot = BuildAST(sourceCode, mainFilePath.string(), slrBuilder);
		std::unordered_set<std::string> parsedFiles;
		parsedFiles.insert(mainFilePath.string());

		ResolveImports(astRoot, parsedFiles, slrBuilder, mainFilePath.parent_path());
		std::cout << "[3.5/5] AST Linking (Imports) resolved." << std::endl;

		// AstPrinter printer;
		// printer.PrintAst(astRoot);

		// TODO регистрировать встроенные функции

		SemanticAnalyzer analyser;
		analyser.Analyze(astRoot);

		if (dumpSchema)
		{
			std::ofstream outFile(dumpFilename);
			if (!outFile.is_open())
			{
				throw std::runtime_error("Не удалось создать или открыть файл: " + dumpFilename);
			}

			SchemaVisualizer visualizer(outFile);
			visualizer.Generate(astRoot);

			outFile.close();
			std::cout << "Схема успешно экспортирована в файл: " << dumpFilename << std::endl;
		}

		if (printSchema)
		{
			ConsoleSchemaVisualizer consoleVisualizer;
			consoleVisualizer.Generate(astRoot);
		}

		const auto chunk = GenerateCode(astRoot);
		RunVM(chunk);

		std::cout << "\nProgram finished." << std::endl;
	}
	catch (const CompilationError& e)
	{
		std::cerr << "\n[!] COMPILATION ERROR:\n"
				  << "File: " << e.location.file << "\n"
				  << "Line: " << e.location.line << ", Col: " << e.location.col << "\n"
				  << "Message: " << e.what() << "\n";
		return 1;
	}
	catch (const std::exception& e)
	{
		std::cerr << "\n[!] COMPILATION ERROR:\n"
				  << e.what() << std::endl;
		return 1;
	}

	return 0;
}

std::string GetSourceCode(const std::string& filePath)
{
	std::string sourceCode = ReadFile(filePath);
	std::cout << "[1/5] File read successfully." << std::endl;
	return sourceCode;
}

SLRTableBuilder SetupSLRTableBuilder(const std::string& prog)
{
	const auto myGrammar = SetupGrammar(prog);
	SLRTableBuilder slrBuilder(myGrammar);
	std::cout << "[2/5] Grammar and SLR table built." << std::endl;

	return slrBuilder;
}

std::shared_ptr<AstNode> BuildAST(const std::string& sourceCode, const std::string& filePath, const SLRTableBuilder& slrBuilder)
{
	Lexer lexer(sourceCode, filePath);
	const Parser parser(slrBuilder, lexer);
	auto astRoot = parser.Parse();
	std::cout << "[3/5] Parsing finished. AST created." << std::endl;

	return astRoot;
}

std::unique_ptr<Chunk> GenerateCode(const std::shared_ptr<AstNode>& astRoot)
{
	CodeGenerator generator;
	auto chunk = generator.Generate(astRoot);
	std::cout << "[4/5] Bytecode generated. Constants: " << chunk->constants.size() << std::endl;

	return chunk;
}

void RunVM(const std::unique_ptr<Chunk>& chunk)
{
	std::cout << "[5/5] Starting Virtual Machine...\n"
			  << std::endl;
	VirtualMachine vm;
	vm.Interpret(chunk);
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

Grammar SetupGrammar(const std::string& exePath)
{
	const auto binaryPath = std::filesystem::absolute(exePath);
	const auto binaryDir = binaryPath.parent_path();
	const auto grammarPath = binaryDir / "../grammar";

	const auto grammar = GrammarReader::Read(grammarPath);

	return grammar;
}

void PrintConsts(const std::unique_ptr<Chunk>& chunk)
{
	for (const auto& val : chunk->constants)
	{
		PrintValue(val);
	}
}

void ResolveImports(
	const std::shared_ptr<AstNode>& astRoot,
	std::unordered_set<std::string>& parsedFiles,
	const SLRTableBuilder& slrBuilder,
	const std::filesystem::path& currentDirPath)
{
	if (!astRoot)
	{
		return;
	}

	const auto programNode = std::dynamic_pointer_cast<CommonNode>(astRoot);
	if (!programNode)
	{
		return;
	}
	if (programNode->type != AstNodeType::PROGRAM)
	{
		return;
	}
	std::vector<std::shared_ptr<AstNode>> resolvedChildren;

	for (const auto& child : programNode->children)
	{
		const auto importNode = std::dynamic_pointer_cast<ImportNode>(child);

		if (!importNode)
		{
			resolvedChildren.push_back(child);
		}
		else
		{
			const auto importPath = currentDirPath / importNode->filePath;
			const auto importPathStr = std::filesystem::absolute(importPath).string();

			if (!parsedFiles.contains(importPathStr))
			{
				parsedFiles.insert(importPathStr);
				std::cout << "      -> Resolving import: " << importNode->filePath << std::endl;

				std::string importedSource = ReadFile(importPathStr);
				auto importedAst = BuildAST(importedSource, importPathStr, slrBuilder);
				ResolveImports(importedAst, parsedFiles, slrBuilder, importPath.parent_path());

				if (const auto importedProgram = std::dynamic_pointer_cast<CommonNode>(importedAst))
				{
					resolvedChildren.insert(
						resolvedChildren.end(),
						importedProgram->children.begin(),
						importedProgram->children.end());
				}
			}
		}
	}

	programNode->children = std::move(resolvedChildren);
}

Params ParseParams(const int argc, char* argv[])
{
	bool printSchema = false;
	bool dumpSchema = false;
	std::string dumpFilename;
	std::string sourceFilePath;

	for (int i = 1; i < argc; ++i)
	{
		const std::string arg = argv[i];

		if (arg == "--print-schema")
		{
			printSchema = true;
		}
		else if (arg == "--dump-schema")
		{
			if (i + 1 >= argc)
			{
				throw std::runtime_error(
					"Критическая ошибка: Флаг --dump-schema требует указания имени файла.\n"
					"Использование: --dump-schema <filename.md>");
			}

			dumpFilename = argv[++i];

			if (dumpFilename.length() < 3
				|| dumpFilename.substr(dumpFilename.length() - 3) != ".md")
			{
				throw std::runtime_error("Ошибка валидации: Файл '" + dumpFilename + "' должен иметь расширение .md");
			}
			dumpSchema = true;
		}
		else
		{
			sourceFilePath = arg;
		}
	}

	if (sourceFilePath.empty())
	{
		throw std::runtime_error(
			"Использование: " + std::string(argv[0]) + " [флаги] <source_file.my_lang>\n"
													   "Доступные флаги:\n"
													   "  --print-schema             Вывести схему конечного автомата в консоль\n"
													   "  --dump-schema <file.md>    Сохранить схему в формате Mermaid (.md)\n");
	}

	return {
		printSchema,
		dumpSchema,
		dumpFilename,
		sourceFilePath,
	};
}