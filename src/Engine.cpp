#include "Engine.h"

#include "CodeGenerator.h"
#include "Common/AstPrinter.h"
#include "Lexer.h"
#include "Parser/Grammar/GrammarReader.h"
#include "Parser/Parser.h"
#include "SematicAnalyzer/SemanticAnalyzer.h"
#include "VirtualMachine.h"
#include "Visualizer/ConsoleSchemaVisualizer.h"
#include "Visualizer/SchemaVisualizer.h"

#include <fstream>
#include <utility>

Grammar SetupGrammar(const std::string& exePath);
std::string ReadFile(const std::string& path);

Engine::Engine(
	std::string exePath,
	CompileFlags flags,
	const bool printAst,
	const bool printLogs)
	: m_exePath(std::move(exePath))
	, m_flags(std::move(flags))
	, m_printAst(printAst)
	, m_printLogs(printLogs)
{
}

void Engine::Compile()
{
	const auto mainFilePath = std::filesystem::absolute(m_exePath);
	const auto sourceCode = GetSourceCode(m_exePath);

	const auto slrBuilder = SetupSLRTableBuilder(m_exePath);
	const auto astRoot = BuildAST(sourceCode, m_exePath, slrBuilder);

	std::unordered_set<std::string> parsedFiles;
	parsedFiles.insert(m_exePath);

	ResolveImports(astRoot, parsedFiles, slrBuilder, mainFilePath.parent_path());

	if (m_printLogs)
	{
		std::cout << "[3.5/5] AST Linking (Imports) resolved." << std::endl;
	}

	if (m_printAst)
	{
		AstPrinter printer;
		printer.PrintAst(astRoot);
	}

	SemanticAnalyzer analyser;
	analyser.Analyze(astRoot);

	if (m_flags.dumpSchema)
	{
		std::ofstream outFile(m_flags.dumpFile);
		if (!outFile.is_open())
		{
			throw std::runtime_error("Не удалось создать или открыть файл: " + m_flags.dumpFile);
		}

		SchemaVisualizer visualizer(outFile);
		visualizer.Generate(astRoot);

		outFile.close();
		std::cout << "Схема успешно экспортирована в файл: " << m_flags.dumpFile << std::endl;
	}
	if (m_flags.printSchema)
	{
		ConsoleSchemaVisualizer consoleVisualizer;
		consoleVisualizer.Generate(astRoot);
	}

	const auto chunk = GenerateCode(astRoot);
	RunVM(chunk);
}

std::string Engine::GetSourceCode(const std::string& filePath) const
{
	std::string sourceCode = ReadFile(filePath);
	if (m_printLogs)
	{
		std::cout << "[1/5] File read successfully." << std::endl;
	}
	return sourceCode;
}

SLRTableBuilder Engine::SetupSLRTableBuilder(const std::string& prog) const
{
	const auto myGrammar = SetupGrammar(prog);
	SLRTableBuilder slrBuilder(myGrammar);
	if (m_printLogs)
	{
		std::cout << "[2/5] Grammar and SLR table built." << std::endl;
	}
	return slrBuilder;
}

std::shared_ptr<AstNode> Engine::BuildAST(const std::string& sourceCode, const std::string& filePath, const SLRTableBuilder& slrBuilder) const
{
	Lexer lexer(sourceCode, filePath);
	const Parser parser(slrBuilder, lexer);
	auto astRoot = parser.Parse();
	if (m_printLogs)
	{
		std::cout << "[3/5] Parsing finished. AST created." << std::endl;
	}
	return astRoot;
}

void Engine::ResolveImports(const std::shared_ptr<AstNode>& astRoot, std::unordered_set<std::string>& parsedFiles, const SLRTableBuilder& slrBuilder, const std::filesystem::__cxx11::path& currentDirPath)
{
	if (!astRoot)
	{
		return;
	}

	const auto programNode = std::dynamic_pointer_cast<CommonNode>(astRoot);
	if (!programNode || programNode->type != AstNodeType::PROGRAM)
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

				if (m_printLogs)
				{
					std::cout << "      -> Resolving import: " << importNode->filePath << std::endl;
				}

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

std::unique_ptr<Chunk> Engine::GenerateCode(const std::shared_ptr<AstNode>& astRoot) const
{
	CodeGenerator generator;
	auto chunk = generator.Generate(astRoot);
	if (m_printLogs)
	{
		std::cout << "[4/5] Bytecode generated." << std::endl;
	}
	return chunk;
}

void Engine::RunVM(const std::unique_ptr<Chunk>& chunk) const
{
	if (m_printLogs)
	{
		std::cout << "[5/5] Starting Virtual Machine...\n"
				  << std::endl;
	}
	VirtualMachine vm;
	vm.Interpret(chunk);
}

Grammar SetupGrammar(const std::string& exePath)
{
	const auto binaryPath = std::filesystem::absolute(exePath);
	const auto binaryDir = binaryPath.parent_path();

	const auto grammar = GrammarReader::Read(GRAMMAR_PATH);
	return grammar;
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
