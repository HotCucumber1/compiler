#pragma once
#include "sql/sql.h"

Value NativePrint(const std::vector<Value>& args);

Value NativeRead(const std::vector<Value>&);

Value NativeFileRead(const std::vector<Value>& args);

Value NativeFileWrite(const std::vector<Value>& args);

Value NativeFileExists(const std::vector<Value>& args);

Value NativePanic(const std::vector<Value>& args);

Value NativeTime(const std::vector<Value>& args);

Value NativeLen(const std::vector<Value>& args);

Value NativeRandomInt(const std::vector<Value>& args);

Value NativeToString(const std::vector<Value>& args);

Value NativeToInt(const std::vector<Value>& args);
