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
				  << "Message: " << e.what() << "\n";
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
					"Критическая ошибка: Флаг --build-module требует указания директории.\n"
					"Использование: --build-module <path/to/output_dir>");
			}
			params.moduleOutputDir = argv[++i];
			params.buildModule = true;
		}
		else if (arg == "--dump-schema")
		{
			if (i + 1 >= argc)
			{
				throw std::runtime_error(
					"Критическая ошибка: Флаг --dump-schema требует указания имени файла.\n"
					"Использование: --dump-schema <filename.md>");
			}
			params.dumpFilename = argv[++i];

			if (params.dumpFilename.length() < 3
				|| params.dumpFilename.substr(params.dumpFilename.length() - 3) != ".md")
			{
				throw std::runtime_error("Ошибка валидации: Файл '" + params.dumpFilename + "' должен иметь расширение .md");
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
			"Использование: " + std::string(argv[0]) + " [флаги] <source_file.stas>\n"
													   "Доступные флаги:\n"
													   "  --print-schema                     Вывести схему конечного автомата в консоль\n"
													   "  --dump-schema <file.md>            Сохранить схему в формате Mermaid (.md)\n"
													   "  --build-module <output_dir>        Скомпилировать файл как модуль и сохранить в указанную директорию\n");
	}

	return params;
}
