#pragma once
#include "../../Type/Value.h"

Value NativeHttpInit(const std::vector<Value>& args);

Value NativeHttpSetOpt(const std::vector<Value>& args);

Value NativeHttpSend(const std::vector<Value>& args);

Value NativeHttpGetStatus(const std::vector<Value>& args);

Value NativeHttpGetResponse(const std::vector<Value>& args);

Value NativeHttpCleanup(const std::vector<Value>& args);
