#pragma once
#include "../../Type/Value.h"

Value NativeDBConnect(const std::vector<Value>& args);

Value NativeDBExecute(const std::vector<Value>& args);

Value NativeDBDisconnect(const std::vector<Value>& args);
