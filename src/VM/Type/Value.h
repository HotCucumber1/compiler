#pragma once

#include "../../../lib/json.hpp"
#include "Object.h"

#include <functional>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>

struct NativeFunction;
using NativeFunctionPtr = std::shared_ptr<NativeFunction>;

struct Function;
using FunctionPtr = std::shared_ptr<Function>;

struct Closure;
using ClosurePtr = std::shared_ptr<Closure>;

struct Array;
using ArrayPtr = std::shared_ptr<Array>;

struct Instance;
using InstancePtr = std::shared_ptr<Instance>;

struct Value : std::variant<std::monostate, int64_t, double, bool, char, HeapObjectPtr, FunctionPtr, ClosurePtr, ArrayPtr, InstancePtr>
{
	using variant::variant;

	explicit operator bool() const
	{
		return std::visit([]<typename T>(T&& v) -> bool {
			using Type = std::decay_t<T>;

			if constexpr (std::is_same_v<Type, std::monostate>)
			{
				return false;
			}
			else if constexpr (std::is_integral_v<Type> || std::is_floating_point_v<Type>)
			{
				return v != 0;
			}
			else if constexpr (std::is_same_v<Type, bool>)
			{
				return v;
			}
			return true; // TODO расписать подробнее
		},
			*this);
	}
};

struct Array final : Object
{
	std::vector<Value> elements;
	Array()
		: Object(ObjectType::ARRAY)
	{
	}
	explicit Array(const size_t size)
		: Object(ObjectType::ARRAY)
		, elements(size)
	{
	}
};

struct Instance final : Object
{
	std::string structName;
	std::unordered_map<std::string, Value> fields;

	explicit Instance(std::string name)
		: Object(ObjectType::INSTANCE)
		, structName(std::move(name))
	{
	}
};

struct NativeFunction final : Object
{
	using NativeFunc = std::function<Value(const std::vector<Value>&)>;

	NativeFunc func;
	std::string name;
	int arity;

	NativeFunction(
		std::string name,
		const int arity,
		NativeFunc func)
		: Object(ObjectType::NATIVE_FUNC)
		, func(std::move(func))
		, name(std::move(name))
		, arity(arity)
	{
	}
};

using NativeFunctionPtr = std::shared_ptr<NativeFunction>;

struct Closure final : Object
{
	FunctionPtr function;
	std::vector<Value> captures;

	Closure(FunctionPtr func, std::vector<Value> caps)
		: Object(ObjectType::CLOSURE)
		, function(std::move(func))
		, captures(std::move(caps))
	{
	}
};

inline bool IsClosure(const Value& value)
{
	return std::visit([]<typename T0>(T0&&) {
		using T = std::decay_t<T0>;
		return std::is_same_v<T, ClosurePtr>;
	},
		value);
}

inline ClosurePtr AsClosure(const Value& value)
{
	if (!IsClosure(value))
	{
		throw std::runtime_error("VM Error: Value is not a closure");
	}
	return std::get<ClosurePtr>(value);
}

inline bool IsNull(const Value& value)
{
	return std::holds_alternative<std::monostate>(value);
}

inline bool IsBool(const Value& value)
{
	return std::holds_alternative<bool>(value);
}

inline Value MakeString(std::string str)
{
	return {
		std::make_shared<String>(std::move(str))
	};
}

inline bool IsHeapObject(const Value& value)
{
	return std::holds_alternative<HeapObjectPtr>(value);
}

inline HeapObjectPtr AsHeapObject(const Value& value)
{
	return std::get<HeapObjectPtr>(value);
}

inline bool IsArray(const Value& value)
{
	if (std::holds_alternative<HeapObjectPtr>(value))
	{
		const auto obj = std::get<HeapObjectPtr>(value);
		return obj && obj->type == ObjectType::ARRAY;
	}
	if (std::holds_alternative<ArrayPtr>(value))
	{
		return true;
	}
	return false;
}

inline bool IsString(const Value& value)
{
	return std::visit([]<typename T0>(T0&& arg) {
		using T = std::decay_t<T0>;
		if constexpr (std::is_same_v<T, HeapObjectPtr>)
		{
			return arg->type == ObjectType::STRING;
		}
		return false;
	},
		value);
}

inline bool IsInstance(const Value& value)
{
	return std::holds_alternative<InstancePtr>(value);
}

inline InstancePtr AsInstance(const Value& value)
{
	if (!IsInstance(value))
	{
		throw std::runtime_error("VM Error: Value is not a struct instance");
	}
	return std::get<InstancePtr>(value);
}

inline std::string AsString(const Value& value)
{
	if (!IsString(value))
	{
		throw std::runtime_error("Value is not a string");
	}

	return std::visit([]<typename T0>(const T0& arg) -> std::string {
		using T = std::decay_t<T0>;
		if constexpr (std::is_same_v<T, HeapObjectPtr>)
		{
			return static_cast<String*>(arg.get())->str;
		}
		return {};
	},
		value);
}

inline bool IsFunction(const Value& value)
{
	return std::visit([]<typename T0>(T0&&) {
		using T = std::decay_t<T0>;
		return std::is_same_v<T, FunctionPtr>;
	},
		value);
}

inline FunctionPtr AsFunc(const Value& value)
{
	if (!IsFunction(value))
	{
		throw std::runtime_error("Value is not a function");
	}
	return std::get<FunctionPtr>(value);
}

inline bool IsNativeFunction(const Value& value)
{
	if (!std::holds_alternative<HeapObjectPtr>(value))
	{
		return false;
	}
	auto obj = std::get<HeapObjectPtr>(value);
	return obj && obj->type == ObjectType::NATIVE_FUNC;
}

inline NativeFunctionPtr AsNativeFunction(const Value& value)
{
	if (!IsNativeFunction(value))
	{
		throw std::runtime_error("Value is not a native function");
	}
	return std::static_pointer_cast<NativeFunction>(std::get<HeapObjectPtr>(value));
}

inline ArrayPtr AsArray(const Value& value)
{
	if (std::holds_alternative<HeapObjectPtr>(value))
	{
		auto obj = std::get<HeapObjectPtr>(value);
		if (obj && obj->type == ObjectType::ARRAY)
		{
			return std::static_pointer_cast<Array>(obj);
		}
	}
	if (std::holds_alternative<ArrayPtr>(value))
	{
		return std::get<ArrayPtr>(value);
	}
	throw std::runtime_error("Value is not an array");
}

inline int64_t AsInt(const Value& value)
{
	return std::visit([]<typename T0>(T0&& arg) -> int64_t {
		using T = std::decay_t<T0>;
		if constexpr (std::is_same_v<T, int64_t>)
		{
			return static_cast<int64_t>(arg);
		}
		throw std::runtime_error("VM Type Error: Expected Int");
	},
		value);
}

inline double AsDouble(const Value& value)
{
	return std::visit([]<typename T0>(T0&& arg) -> double {
		using T = std::decay_t<T0>;
		if constexpr (std::is_same_v<T, double>)
		{
			return static_cast<double>(arg);
		}
		throw std::runtime_error("VM Type Error: Expected Double");
	},
		value);
}

inline bool AsBool(const Value& value)
{
	return std::visit([]<typename T0>(T0&& arg) -> bool {
		using T = std::decay_t<T0>;
		if constexpr (std::is_same_v<T, bool>)
		{
			return static_cast<bool>(arg);
		}
		throw std::runtime_error("VM Type Error: Expected bool");
	},
		value);
}

inline bool operator==(const Value& lhs, const Value& rhs)
{
	return std::visit([]<typename T0, typename T1>(const T0& l, const T1& r) -> bool {
		using LType = std::decay_t<T0>;
		using RType = std::decay_t<T1>;

		if constexpr (std::is_integral_v<LType> && std::is_floating_point_v<RType>)
		{
			return static_cast<double>(l) == static_cast<double>(r);
		}
		else if constexpr (std::is_floating_point_v<LType> && std::is_integral_v<RType>)
		{
			return static_cast<double>(l) == static_cast<double>(r);
		}

		else if constexpr (std::is_same_v<LType, RType>)
		{
			if constexpr (std::is_same_v<LType, HeapObjectPtr>)
			{
				if (l->type == ObjectType::STRING && r->type == ObjectType::STRING)
				{
					return static_cast<String*>(l.get())->str == static_cast<String*>(r.get())->str;
				}
				return l == r;
			}
			else if constexpr (std::is_same_v<LType, std::monostate>)
			{
				return true;
			}
			else
			{
				return l == r;
			}
		}

		else
		{
			return false;
		}
	},
		lhs, rhs);
}

// OP_NOT_EQUAL
inline bool operator!=(const Value& lhs, const Value& rhs)
{
	return !(lhs == rhs);
}

// OP_ADD
inline Value operator+(const Value& lhs, const Value& rhs) // TODO добавить поддержку сложения char и str
{
	return std::visit([&]<typename T0, typename T1>(T0&& l, T1&& r) -> Value {
		using LType = std::decay_t<T0>;
		using RType = std::decay_t<T1>;

		if constexpr (std::is_same_v<LType, bool> || std::is_same_v<RType, bool>)
		{
			throw std::runtime_error("Cannot add bool values");
		}
		else if constexpr (std::is_same_v<LType, HeapObjectPtr> || std::is_same_v<RType, HeapObjectPtr>)
		{
			if (IsString(l) && IsString(r))
			{
				const auto& lStr = AsString(l);
				const auto& rStr = AsString(r);
				return MakeString(lStr + rStr);
			}
		}
		else if constexpr (std::is_integral_v<LType> && std::is_integral_v<RType>)
		{
			using CommonInt = std::conditional_t<std::is_same_v<LType, int64_t> || std::is_same_v<RType, int64_t>, int64_t, char>;
			return static_cast<CommonInt>(l) + static_cast<CommonInt>(r);
		}
		else if constexpr (std::is_same_v<LType, FunctionPtr> || std::is_same_v<RType, FunctionPtr>)
		{
			throw std::runtime_error("Cannot add function");
		}
		else if constexpr (std::is_same_v<LType, ArrayPtr> || std::is_same_v<RType, ArrayPtr>)
		{
			throw std::runtime_error("Cannot add array"); // TODO
		}
		else if constexpr (std::is_same_v<LType, std::monostate> || std::is_same_v<RType, std::monostate>)
		{
			throw std::runtime_error("Cannot add null"); // TODO
		}
		else if constexpr (std::is_same_v<LType, InstancePtr> || std::is_same_v<RType, InstancePtr>)
		{
			throw std::runtime_error("Cannot add structs"); // TODO
		}
		else if constexpr (std::is_same_v<LType, ClosurePtr> || std::is_same_v<RType, ClosurePtr>)
		{
			throw std::runtime_error("Cannot add closure"); // TODO
		}
		else if constexpr (std::is_same_v<LType, double> || std::is_same_v<RType, double>)
		{
			return static_cast<double>(l) + static_cast<double>(r);
		}
		throw std::runtime_error("Unsupported add arguments combination");
	},
		lhs, rhs);
}

// OP_SUB
inline Value operator-(const Value& lhs, const Value& rhs)
{
	return std::visit([&]<typename T0, typename T1>(T0&& l, T1&& r) -> Value {
		using LType = std::decay_t<T0>;
		using RType = std::decay_t<T1>;

		if constexpr (std::is_same_v<LType, bool> || std::is_same_v<RType, bool>)
		{
			throw std::runtime_error("Cannot sub bool values");
		}
		else if constexpr (std::is_same_v<LType, HeapObjectPtr> || std::is_same_v<RType, HeapObjectPtr>)
		{
			if (IsString(l) || IsString(r))
			{
				throw std::runtime_error("Cannot sub strings values");
			}
		}
		else if constexpr (std::is_integral_v<LType> && std::is_integral_v<RType>)
		{
			using CommonInt = std::conditional_t<std::is_same_v<LType, int64_t> || std::is_same_v<RType, int64_t>, int64_t, char>;
			return static_cast<CommonInt>(l) - static_cast<CommonInt>(r);
		}
		else if constexpr (std::is_same_v<LType, FunctionPtr> || std::is_same_v<RType, FunctionPtr>)
		{
			throw std::runtime_error("Cannot sub function");
		}
		else if constexpr (std::is_same_v<LType, ArrayPtr> || std::is_same_v<RType, ArrayPtr>)
		{
			throw std::runtime_error("Cannot sub array"); // TODO
		}
		else if constexpr (std::is_same_v<LType, std::monostate> || std::is_same_v<RType, std::monostate>)
		{
			throw std::runtime_error("Cannot sub null"); // TODO
		}
		else if constexpr (std::is_same_v<LType, InstancePtr> || std::is_same_v<RType, InstancePtr>)
		{
			throw std::runtime_error("Cannot sub structs"); // TODO
		}
		else if constexpr (std::is_same_v<LType, ClosurePtr> || std::is_same_v<RType, ClosurePtr>)
		{
			throw std::runtime_error("Cannot sub closure"); // TODO
		}
		else if constexpr (std::is_same_v<LType, double> || std::is_same_v<RType, double>)
		{
			return static_cast<double>(l) - static_cast<double>(r);
		}
		throw std::runtime_error("Unsupported sub arguments combination");
	},
		lhs, rhs);
}

// OP_MULTI
inline Value operator*(const Value& lhs, const Value& rhs)
{
	return std::visit([&]<typename T0, typename T1>(T0&& l, T1&& r) -> Value {
		using LType = std::decay_t<T0>;
		using RType = std::decay_t<T1>;

		if constexpr (std::is_same_v<LType, bool> || std::is_same_v<RType, bool>)
		{
			throw std::runtime_error("Cannot multiply bool values");
		}
		else if constexpr (std::is_same_v<LType, HeapObjectPtr> || std::is_same_v<RType, HeapObjectPtr>)
		{
			if (IsString(l) || IsString(r))
			{
				throw std::runtime_error("Cannot multiply strings values");
			}
		}
		else if constexpr (std::is_integral_v<LType> && std::is_integral_v<RType>)
		{
			using CommonInt = std::conditional_t<std::is_same_v<LType, int64_t> || std::is_same_v<RType, int64_t>, int64_t, char>;
			return static_cast<CommonInt>(l) * static_cast<CommonInt>(r);
		}
		else if constexpr (std::is_same_v<LType, FunctionPtr> || std::is_same_v<RType, FunctionPtr>)
		{
			throw std::runtime_error("Cannot multiply function");
		}
		else if constexpr (std::is_same_v<LType, ArrayPtr> || std::is_same_v<RType, ArrayPtr>)
		{
			throw std::runtime_error("Cannot multiply array"); // TODO
		}
		else if constexpr (std::is_same_v<LType, std::monostate> || std::is_same_v<RType, std::monostate>)
		{
			throw std::runtime_error("Cannot multiply null"); // TODO
		}
		else if constexpr (std::is_same_v<LType, InstancePtr> || std::is_same_v<RType, InstancePtr>)
		{
			throw std::runtime_error("Cannot multiply structs"); // TODO
		}
		else if constexpr (std::is_same_v<LType, ClosurePtr> || std::is_same_v<RType, ClosurePtr>)
		{
			throw std::runtime_error("Cannot multiply closure"); // TODO
		}
		else if constexpr (std::is_same_v<LType, double> || std::is_same_v<RType, double>)
		{
			return static_cast<double>(l) * static_cast<double>(r);
		}
		throw std::runtime_error("Unsupported multi arguments combination");
	},
		lhs, rhs);
}

// OP_DIV
inline Value operator/(const Value& lhs, const Value& rhs)
{
	return std::visit([&]<typename T0, typename T1>(T0&& l, T1&& r) -> Value {
		using LType = std::decay_t<T0>;
		using RType = std::decay_t<T1>;

		if constexpr (std::is_same_v<LType, bool> || std::is_same_v<RType, bool>)
		{
			throw std::runtime_error("Cannot div bool values");
		}
		else if constexpr (std::is_same_v<LType, HeapObjectPtr> || std::is_same_v<RType, HeapObjectPtr>)
		{
			if (IsString(l) || IsString(r))
			{
				throw std::runtime_error("Cannot div strings values");
			}
		}
		else if constexpr (std::is_integral_v<LType> && std::is_integral_v<RType>)
		{
			using CommonInt = std::conditional_t<std::is_same_v<LType, int64_t> || std::is_same_v<RType, int64_t>, int64_t, char>;
			if (r == 0)
			{
				throw std::runtime_error("Division by zero");
			}
			return static_cast<CommonInt>(l) / static_cast<CommonInt>(r);
		}
		else if constexpr (std::is_same_v<LType, FunctionPtr> || std::is_same_v<RType, FunctionPtr>)
		{
			throw std::runtime_error("Cannot div function");
		}
		else if constexpr (std::is_same_v<LType, ArrayPtr> || std::is_same_v<RType, ArrayPtr>)
		{
			throw std::runtime_error("Cannot div array"); // TODO
		}
		else if constexpr (std::is_same_v<LType, std::monostate> || std::is_same_v<RType, std::monostate>)
		{
			throw std::runtime_error("Cannot div null"); // TODO
		}
		else if constexpr (std::is_same_v<LType, InstancePtr> || std::is_same_v<RType, InstancePtr>)
		{
			throw std::runtime_error("Cannot div structs"); // TODO
		}
		else if constexpr (std::is_same_v<LType, ClosurePtr> || std::is_same_v<RType, ClosurePtr>)
		{
			throw std::runtime_error("Cannot div closure"); // TODO
		}
		else if constexpr (std::is_same_v<LType, double> || std::is_same_v<RType, double>)
		{
			return static_cast<double>(l) / static_cast<double>(r);
		}
		throw std::runtime_error("Unsupported div arguments combination");
	},
		lhs, rhs);
}

// OP_MOD
inline Value operator%(const Value& lhs, const Value& rhs)
{
	return std::visit([&]<typename T0, typename T1>(T0&& l, T1&& r) -> Value {
		using LType = std::decay_t<T0>;
		using RType = std::decay_t<T1>;

		if constexpr (std::is_same_v<LType, bool> || std::is_same_v<RType, bool>)
		{
			throw std::runtime_error("Cannot mod bool values");
		}
		else if constexpr (std::is_same_v<LType, HeapObjectPtr> || std::is_same_v<RType, HeapObjectPtr>)
		{
			if (IsString(l) || IsString(r))
			{
				throw std::runtime_error("Cannot mod strings values");
			}
		}
		else if constexpr (std::is_integral_v<LType> && std::is_integral_v<RType>)
		{
			using CommonInt = std::conditional_t<std::is_same_v<LType, int64_t> || std::is_same_v<RType, int64_t>, int64_t, char>;
			return static_cast<CommonInt>(l) % static_cast<CommonInt>(r);
		}
		else if constexpr (std::is_same_v<LType, double> || std::is_same_v<RType, double>)
		{
			throw std::runtime_error("Cannot mod non-integers");
		}
		throw std::runtime_error("Unsupported mod arguments combination");
	},
		lhs, rhs);
}

// OP_NEG
inline Value operator-(const Value& value)
{
	return std::visit([]<typename T>(T&& v) -> Value {
		using Type = std::decay_t<T>;

		if constexpr (std::is_same_v<Type, bool>)
		{
			throw std::runtime_error("Cannot apply unary minus to bool");
		}
		else if constexpr (std::is_same_v<Type, HeapObjectPtr>)
		{
			if (IsString(v))
			{
				throw std::runtime_error("Cannot apply unary minus to string values");
			}
		}
		else if constexpr (std::is_same_v<Type, char>)
		{
			return -static_cast<int64_t>(v);
		}
		else if constexpr (std::is_same_v<Type, int64_t> || std::is_same_v<Type, double>)
		{
			return -v;
		}
		throw std::runtime_error("Unsupported type for unary minus");
	},
		value);
}

inline void PrintValue(const Value& value, bool newLine = true)
{
	try
	{
		std::visit([]<typename T0>(T0&& arg) {
			using T = std::decay_t<T0>;
			if constexpr (std::is_same_v<T, std::monostate>)
			{
				std::cout << "null";
			}
			else if (IsString(arg))
			{
				const auto& str = AsString(arg);
				std::cout << str;
			}
			else if (IsArray(arg))
			{
				bool isStart = true;
				const auto arr = AsArray(arg)->elements;
				std::cout << "Array (" << arr.size() << ") => [";
				for (const auto& val : arr)
				{
					if (!isStart)
					{
						std::cout << ", ";
					}
					else
					{
						isStart = false;
					}
					PrintValue(val, false);
				}
				std::cout << "]";
			}
			else if (IsBool(arg))
			{
				const auto isTrue = AsBool(arg);
				const auto val = isTrue ? "true" : "false";
				std::cout << val;
			}
			else if (IsInstance(arg))
			{
				const auto inst = AsInstance(arg);
				std::cout << "<instance of " << inst->structName << ">";
			}
			else if (IsClosure(arg))
			{
				const auto closure = AsClosure(arg);
				std::cout << "<closure capturing " << closure->captures.size() << " vars>";
			}
			else
			{
				std::cout << arg;
			}
		},
			value);
		if (newLine)
		{
			std::cout << std::endl;
		}
	}
	catch (const std::bad_variant_access& e)
	{
		std::cout << "Error accessing variant: " << e.what() << std::endl;
	}
}

inline Value Read()
{
	std::string input;
	if (!std::getline(std::cin, input))
	{
		if (std::cin.eof())
		{
			return {};
		}
		throw std::runtime_error("Failed to read input");
	}
	return MakeString(input);
}
