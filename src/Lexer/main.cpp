#include "Lexer.h"

#include <fstream>
#include <iostream>

int main()
{
	try
	{
		std::ifstream file("input_test/big_prog");
		Lexer lexer(file);

		std::cout << "-------------------------------------------------" << std::endl;
		while (true)
		{
			if (lexer.IsEmpty())
			{
				break;
			}
			const auto token = lexer.GetToken();
			std::cout << "Value: " << token.value << std::endl
					  << "Line:  " << token.line << std::endl
					  << "Pos:   " << token.pos << std::endl
					  << "Token: " << TOKEN_NAMES.at(token.type)
					  << std::endl;
			std::cout << "-------------------------------------------------" << std::endl;

			if (token.type == TokenType::ERROR)
			{
				break;
			}
		}
		std::cout << "Reading ended" << std::endl;
	}
	catch (const std::exception& exception)
	{
		std::cout << exception.what() << std::endl;
		return 1;
	}
}