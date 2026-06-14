#include "Engine/Engine.h"

struct Params
{
	bool printSchema = false;
	bool dumpSchema = false;
	bool buildModule = false;
	std::string moduleOutputDir;
	std::string dumpFilename;
	std::string sourceFilePath;
};

Params ParseParams(int argc, char* argv[]);

int main(const int argc, char* argv[])
{
	try
	{
		const auto [printSchema,
			dumpSchema,
			buildModule,
			moduleOutputDir,
			dumpFilename,
			sourceFilePath]
			= ParseParams(argc, argv);

		Engine engine(
			sourceFilePath,
			{
				printSchema,
				dumpSchema,
				buildModule,
				dumpFilename,
				moduleOutputDir,
			},
			false,
			false);
		engine.Compile();

		std::cout << "\nProgram finished." << std::endl;
	}
	catch (const CompilationError& e)
	{
		std::cerr << "\n[!] COMPILATION ERROR:\n"
				  << "File: " << e.location.file << "\n"
				  << "Line: " << e.location.line << ", Col: " << e.location.col << "\n"
				  << "Message: \n"
				  << e.what() << "\n";
		return 1;
	}
	catch (const std::exception& e)
	{
		std::cerr << "\nError:\n"
				  << e.what() << std::endl;
		return 1;
	}

	return 0;
}

Params ParseParams(const int argc, char* argv[])
{
	Params params;

	for (int i = 1; i < argc; ++i)
	{
		const std::string arg = argv[i];

		if (arg == "--print-schema")
		{
			params.printSchema = true;
		}
		else if (arg == "--build-module")
		{
			if (i + 1 >= argc)
			{
				throw std::runtime_error(
					"Flag --build-module requires an output directory specification.\n"
					"Usage: --build-module <path/to/output_dir>");
			}
			params.moduleOutputDir = argv[++i];
			params.buildModule = true;
		}
		else if (arg == "--dump-schema")
		{
			if (i + 1 >= argc)
			{
				throw std::runtime_error(
					"Flag --dump-schema requires an output filename specification.\n"
					"Usage: --dump-schema <filename.md>");
			}
			params.dumpFilename = argv[++i];

			if (params.dumpFilename.length() < 3
				|| params.dumpFilename.substr(params.dumpFilename.length() - 3) != ".md")
			{
				throw std::runtime_error("Validation error: File '" + params.dumpFilename + "' must have a .md extension.");
			}
			params.dumpSchema = true;
		}
		else
		{
			params.sourceFilePath = arg;
		}
	}

	if (params.sourceFilePath.empty())
	{
		throw std::runtime_error(
			"Usage: " + std::string(argv[0]) + " [flags] <source_file.stas>\n"
											   "Available flags:\n"
											   "  --print-schema                     Print finite state machine schema to console\n"
											   "  --dump-schema <file.md>            Save schema in Mermaid format (.md)\n"
											   "  --build-module <output_dir>        Compile file as a module and save to specified directory\n");
	}

	return params;
}
