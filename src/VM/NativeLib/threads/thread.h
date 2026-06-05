#pragma once
#include "Type/Value.h"

#include <thread>

struct ThreadState
{
	std::thread worker;
	Value result;
	std::string errorMsg;
	std::atomic<bool> isDone;
};

static int64_t g_nextThreadId = 1;
static std::unordered_map<int64_t, std::unique_ptr<ThreadState>> g_threads;
static std::mutex g_threadsMutex;
