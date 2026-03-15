#pragma once
#include "../Type/Object.h"

#include <unordered_map>

class StringPool
{
public:
	HeapObjectPtr Intern(std::string content);

private:
	std::unordered_map<std::string_view, std::weak_ptr<String>> m_pool;
};
