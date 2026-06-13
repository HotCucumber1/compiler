#pragma once
#include "../../Type/Value.h"

Value NativeJsonToMap(const std::vector<Value>& args);
Value NativeMapToJson(const std::vector<Value>& args);
Value NativeJsonEscape(const std::vector<Value>& args);
