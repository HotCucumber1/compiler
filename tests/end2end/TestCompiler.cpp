#include "../../src/Engine.h"
#include "../Common/StdOutRedirector.h"
#include "../Common/TestParser.h"
#include <catch2/catch_all.hpp>

TEST_CASE("End-to-End Compiler Tests", "[e2e]")
{
	auto files = GetTestFiles("data/");

	if (files.empty())
	{
		std::cerr << "Empty directory" << std::endl;
		return;
	}

	std::string filePath = GENERATE_REF(Catch::Generators::from_range(files));

	DYNAMIC_SECTION("Testing file: " << filePath)
	{
		const auto& [expectedOutput, expectedError, hasError] = ParseTestConfig(filePath);

		StdOutRedirector redirector{};
		bool executionFailed = false;
		std::string actualErrorMsg;

		try
		{
			Engine engine(filePath, {}, false, false);
			engine.Compile();
		}
		catch (const std::exception& e)
		{
			executionFailed = true;
			actualErrorMsg = e.what();
		}

		if (hasError)
		{
			REQUIRE(executionFailed == true);
			REQUIRE(actualErrorMsg.find(expectedError) != std::string::npos);
		}
		else
		{
			if (executionFailed)
			{
				FAIL("Unexpected error during execution: " << actualErrorMsg);
			}
			else
			{
				const auto& actualOutput = redirector.GetOutput();
				REQUIRE(actualOutput == expectedOutput);
			}
		}
	}
}
