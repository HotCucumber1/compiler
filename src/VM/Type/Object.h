#pragma once
#include <memory>
#include <string>

enum class ObjectType
{
	STRING,
	OBJECT,
};

struct Object
{
	ObjectType type;
	explicit Object(const ObjectType t)
		: type(t)
	{
	}
	virtual ~Object() = default;
};

struct String final : Object
{
	std::string str;
	explicit String(std::string s)
		: Object(ObjectType::STRING)
		, str(std::move(s))
	{
	}
};

using HeapObjectPtr = std::shared_ptr<Object>;
