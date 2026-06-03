#pragma once
#include "../Type/Value.h"

Value NativePrint(const std::vector<Value>& args);

Value NativeRead(const std::vector<Value>&);

Value NativeFileRead(const std::vector<Value>& args);

Value NativeFileWrite(const std::vector<Value>& args);

Value NativeFileExists(const std::vector<Value>& args);

Value NativePanic(const std::vector<Value>& args);