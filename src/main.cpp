#include <iostream>

int main()
{
	try
	{
		std::cout << "Hello world" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}