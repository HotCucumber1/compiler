#include "Engine.h"

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
		const auto [printSchema,
			dumpSchema,
			dumpFilename,
			sourceFilePath]
			= ParseParams(argc, argv);

		Engine engine(
			sourceFilePath,
			{
				printSchema,
				dumpSchema,
				dumpFilename,
			},
			false,
			true);
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
