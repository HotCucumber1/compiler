#pragma once
#include "../../Type/Value.h"

Value NativeHttpServerStart(const std::vector<Value>& args);
Value NativeHttpServerStop(const std::vector<Value>& args);
Value NativeHttpServerAccept(const std::vector<Value>& args);
Value NativeHttpRequestMethod(const std::vector<Value>& args);
Value NativeHttpRequestPath(const std::vector<Value>& args);
Value NativeHttpRequestHeader(const std::vector<Value>& args);
Value NativeHttpRequestBody(const std::vector<Value>& args);
Value NativeHttpRequestRespond(const std::vector<Value>& args);
