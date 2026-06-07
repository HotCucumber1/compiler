#pragma once
#include "Parser/SLRTableBuilder.h"
#include "Type/Chunk.h"

#include <filesystem>
#include <string>
#include <unordered_set>

struct CompileFlags
{
	bool printSchema;
	bool dumpSchema;
	std::string dumpFile;
};

class Engine
{
public:
	explicit Engine(
		std::string exePath,
		CompileFlags flags,
		bool printAst = false,
		bool printLogs = false);

	void Compile();

private:
	std::string GetSourceCode(const std::string& filePath) const;

	SLRTableBuilder SetupSLRTableBuilder(const std::string& prog) const;

	std::shared_ptr<AstNode> BuildAST(
		const std::string& sourceCode,
		const std::string& filePath,
		const SLRTableBuilder& slrBuilder) const;

	void ResolveImports(
		const std::shared_ptr<AstNode>& astRoot,
		std::unordered_set<std::string>& parsedFiles,
		const SLRTableBuilder& slrBuilder,
		const std::filesystem::__cxx11::path& currentDirPath);

	std::unique_ptr<Chunk> GenerateCode(const std::shared_ptr<AstNode>& astRoot) const;

	void RunVM(const std::unique_ptr<Chunk>& chunk) const;

private:
	std::string m_exePath;
	CompileFlags m_flags;
	bool m_printAst;
	bool m_printLogs;
};
