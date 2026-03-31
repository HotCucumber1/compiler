#pragma once

#include "Object.h"
#include <iostream>
#include <stdexcept>
#include <variant>

struct Function;
using FunctionPtr = std::shared_ptr<Function>;

using Value = std::variant<
	int64_t,
	double,
	bool,
	char,
	HeapObjectPtr,
	FunctionPtr>;

inline Value MakeString(std::string str)
{
	return {
		std::make_shared<String>(std::move(str))
	};
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

inline int AsInt(const Value& value)
{
	return std::visit([]<typename T0>(T0&& arg) -> int {
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
		else if constexpr (std::is_same_v<LType, double> || std::is_same_v<RType, double>)
		{
			return static_cast<double>(l) + static_cast<double>(r);
		}
		throw std::runtime_error("Unsupported combination");
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
		else if constexpr (std::is_same_v<LType, double> || std::is_same_v<RType, double>)
		{
			return static_cast<double>(l) - static_cast<double>(r);
		}
		throw std::runtime_error("Unsupported combination");
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
		else if constexpr (std::is_same_v<LType, double> || std::is_same_v<RType, double>)
		{
			return static_cast<double>(l) * static_cast<double>(r);
		}
		throw std::runtime_error("Unsupported combination");
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
		else if constexpr (std::is_same_v<LType, double> || std::is_same_v<RType, double>)
		{
			return static_cast<double>(l) / static_cast<double>(r);
		}
		throw std::runtime_error("Unsupported combination");
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
		throw std::runtime_error("Unsupported combination");
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

// OP_NOT
inline bool operator!(const Value& value)
{
	return std::visit([]<typename T>(T&& v) -> bool {
		using Type = std::decay_t<T>;

		if constexpr (std::is_integral_v<Type> || std::is_same_v<Type, double>)
		{
			return v == 0;
		}
		if constexpr (std::is_same_v<Type, bool>)
		{
			return !v;
		}
		throw std::runtime_error("Unsupported type for not operation");
	},
		value);
}

inline void PrintValue(const Value& value)
{
	try
	{
		std::visit([](auto&& arg) {
			if (IsString(arg))
			{
				const auto& str = AsString(arg);
				std::cout << str;
			}
			else
			{
				std::cout << arg;
			}
		},
			value);
		std::cout << std::endl;
	}
	catch (const std::bad_variant_access& e)
	{
		std::cout << "Error accessing variant: " << e.what() << std::endl;
	}
}
