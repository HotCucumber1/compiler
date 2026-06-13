#include "Engine.h"

#include "../Common/AstPrinter.h"
#include "../Parser/Grammar/GrammarReader.h"
#include "../Parser/Parser.h"
#include "../SematicAnalyzer/SemanticAnalyzer.h"
#include "../Visualizer/ConsoleSchemaVisualizer.h"
#include "../Visualizer/SchemaVisualizer.h"
#include "CodeGenerator.h"
#include "Lexer.h"
#include "VirtualMachine.h"

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
	const auto mainFilePath = std::filesystem::weakly_canonical(m_exePath);
	const auto sourceCode = GetSourceCode(m_exePath);

	const auto slrBuilder = SetupSLRTableBuilder(m_exePath);
	const auto astRoot = BuildAST(sourceCode, m_exePath, slrBuilder);

	std::unordered_set<std::string> parsedFiles;
	parsedFiles.insert(m_exePath);

	std::vector<std::string> importStack;
	importStack.push_back(mainFilePath);
	ResolveImports(parsedFiles, importStack, astRoot, slrBuilder, mainFilePath.parent_path());

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

	CodeGenerator generator;
	const auto chunk = generator.Generate(astRoot);

	if (m_flags.buildModule)
	{
		auto sourcePath = std::filesystem::path(m_exePath);
		auto outDir = std::filesystem::path(m_flags.moduleOutputDir);
		if (!std::filesystem::exists(outDir))
		{
			std::filesystem::create_directories(outDir);
		}

		auto stasmPath = outDir / sourcePath.filename();
		stasmPath.replace_extension(".stasm");
		analyser.ExportModuleMetadata(stasmPath.string());

		auto stasbPath = outDir / sourcePath.filename();
		stasbPath.replace_extension(".stasbc");
		generator.ExportModuleBytecode(stasbPath.string());

		if (m_printLogs)
		{
			std::cout << "[5/5] Module successfully built!\n      Metadata: "
					  << stasmPath.string() << "\n      Bytecode: " << stasbPath.string() << std::endl;
		}

		return;
	}
	if (m_printLogs)
	{
		std::cout << "[4/5] Bytecode generated." << std::endl;
	}
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

void Engine::ResolveImports(
	std::unordered_set<std::string>& parsedFiles,
	std::vector<std::string>& importStack,
	const std::shared_ptr<AstNode>& astRoot,
	const SLRTableBuilder& slrBuilder,
	const std::filesystem::path& currentDirPath)
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
			continue;
		}
		const auto importPath = currentDirPath / importNode->filePath;
		const auto importPathStr = std::filesystem::weakly_canonical(importPath).string();

		if (importPathStr.ends_with(".stasm"))
		{
			importNode->filePath = importPathStr;
			m_importedModules.push_back(importPathStr);
			resolvedChildren.push_back(child);
			continue;
		}
		if (std::ranges::find(importStack, importPathStr) != importStack.end())
		{
			std::string cycleTrace;
			for (const auto& path : importStack)
			{
				cycleTrace += path + "\n -> ";
			}
			cycleTrace += importPathStr;

			throw std::runtime_error("Compile error: Circular import detected!\n" + cycleTrace);
		}

		if (!parsedFiles.contains(importPathStr))
		{
			parsedFiles.insert(importPathStr);
			importStack.push_back(importPathStr);

			if (m_printLogs)
			{
				std::cout << "      -> Resolving import: " << importNode->filePath << std::endl;
			}

			const auto importedSource = ReadFile(importPathStr);
			auto importedAst = BuildAST(importedSource, importPathStr, slrBuilder);
			ResolveImports(parsedFiles, importStack, importedAst, slrBuilder, importPath.parent_path());

			importStack.pop_back();
			if (const auto importedProgram = std::dynamic_pointer_cast<CommonNode>(importedAst))
			{
				resolvedChildren.insert(
					resolvedChildren.end(),
					importedProgram->children.begin(),
					importedProgram->children.end());
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
		std::cout << "[5/5] Starting Virtual Machine..." << std::endl;
	}

	VirtualMachine vm;
	CodeGenerator generator;
	for (const auto& modulePath : m_importedModules)
	{
		auto stasbPath = std::filesystem::path(modulePath);
		stasbPath.replace_extension(".stasbc");
		auto modChunk = generator.ImportModuleBytecode(stasbPath.string());
		vm.Interpret(modChunk);
	}
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
