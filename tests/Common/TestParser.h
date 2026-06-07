#pragma once
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct TestConfig
{
	std::string expectedOutput;
	std::string expectedError;
	bool hasError = false;
};

inline TestConfig ParseTestConfig(const std::string& filePath)
{
	TestConfig config;
	std::ifstream file(filePath);
	std::string line;

	while (std::getline(file, line))
	{
		const auto start = line.find_first_not_of(" \t");
		if (start == std::string::npos)
		{
			continue;
		}
		line = line.substr(start);

		if (line.starts_with("// EXPECT: "))
		{
			if (!config.expectedOutput.empty())
			{
				config.expectedOutput += "\n";
			}
			config.expectedOutput += line.substr(11);
		}
		else if (line.starts_with("// ERROR: "))
		{
			config.expectedError = line.substr(10);
			config.hasError = true;
		}
	}
	return config;
}

inline std::vector<std::string> GetTestFiles(const std::string& directory)
{
	std::vector<std::string> files;
	if (!fs::exists(directory))
	{
		return files;
	}

	for (const auto& entry : fs::recursive_directory_iterator(directory))
	{
		if (entry.is_regular_file()
			&& entry.path().extension() == ".stas")
		{
			files.push_back(entry.path().string());
		}
	}
	return files;
}
