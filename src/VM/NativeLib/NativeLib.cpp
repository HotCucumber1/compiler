#include "NativeLib.h"

Value NativePrint(const std::vector<Value>& args)
{
	for (const auto& arg : args)
	{
		PrintValue(arg);
	}
	return {};
}

Value NativeRead(const std::vector<Value>&)
{
	return Read();
}
