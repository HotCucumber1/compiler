#include "StringPool.h"

HeapObjectPtr StringPool::Intern(std::string content)
{
	auto it = m_pool.find(content);
	if (it != m_pool.end())
	{
		if (auto obj = it->second.lock())
		{
			return obj;
		}
		m_pool.erase(it);
	}

	auto newObj = std::make_shared<String>(std::move(content));
	m_pool[newObj->str] = newObj;
	return newObj;
}
