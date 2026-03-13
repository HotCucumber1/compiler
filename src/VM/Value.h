#pragma once
#include "Type/Object.h"

#include <iostream>
#include <stdexcept>
#include <variant>

using Value = std::variant<
	int64_t,
	double,
	bool,
	char,
	HeapObjectPtr>;

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

inline std::string AsString(const Value& value)
{
	if (!IsString(value))
	{
		throw std::invalid_argument("Value is not a string");
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

// OP_ADD
inline Value operator+(const Value& lhs, const Value& rhs) // TODO добавить поддержку сложения char и str
{
	return std::visit([&]<typename T0, typename T1>(T0&& l, T1&& r) -> Value {
		using LType = std::decay_t<T0>;
		using RType = std::decay_t<T1>;

		if constexpr (std::is_same_v<LType, bool> || std::is_same_v<RType, bool>)
		{
			throw std::invalid_argument("Cannot add bool values");
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
		else if constexpr (std::is_same_v<LType, double> || std::is_same_v<RType, double>)
		{
			return static_cast<double>(l) + static_cast<double>(r);
		}
		else
		{
			throw std::invalid_argument("Unsupported combination");
		}
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
			throw std::invalid_argument("Cannot sub bool values");
		}
		else if constexpr (std::is_same_v<LType, HeapObjectPtr> || std::is_same_v<RType, HeapObjectPtr>)
		{
			if (IsString(l) || IsString(r))
			{
				throw std::invalid_argument("Cannot sub strings values");
			}
		}
		else if constexpr (std::is_integral_v<LType> && std::is_integral_v<RType>)
		{
			using CommonInt = std::conditional_t<std::is_same_v<LType, int64_t> || std::is_same_v<RType, int64_t>, int64_t, char>;
			return static_cast<CommonInt>(l) - static_cast<CommonInt>(r);
		}
		else if constexpr (std::is_same_v<LType, double> || std::is_same_v<RType, double>)
		{
			return static_cast<double>(l) - static_cast<double>(r);
		}
		else
		{
			throw std::invalid_argument("Unsupported combination");
		}
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
			throw std::invalid_argument("Cannot multiply bool values");
		}
		else if constexpr (std::is_same_v<LType, HeapObjectPtr> || std::is_same_v<RType, HeapObjectPtr>)
		{
			if (IsString(l) || IsString(r))
			{
				throw std::invalid_argument("Cannot multiply strings values");
			}
		}
		else if constexpr (std::is_integral_v<LType> && std::is_integral_v<RType>)
		{
			using CommonInt = std::conditional_t<std::is_same_v<LType, int64_t> || std::is_same_v<RType, int64_t>, int64_t, char>;
			return static_cast<CommonInt>(l) * static_cast<CommonInt>(r);
		}
		else if constexpr (std::is_same_v<LType, double> || std::is_same_v<RType, double>)
		{
			return static_cast<double>(l) * static_cast<double>(r);
		}
		else
		{
			throw std::invalid_argument("Unsupported combination");
		}
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
			throw std::invalid_argument("Cannot div bool values");
		}
		else if constexpr (std::is_same_v<LType, HeapObjectPtr> || std::is_same_v<RType, HeapObjectPtr>)
		{
			if (IsString(l) || IsString(r))
			{
				throw std::invalid_argument("Cannot div strings values");
			}
		}
		else if constexpr (std::is_integral_v<LType> && std::is_integral_v<RType>)
		{
			using CommonInt = std::conditional_t<std::is_same_v<LType, int64_t> || std::is_same_v<RType, int64_t>, int64_t, char>;
			return static_cast<CommonInt>(l) / static_cast<CommonInt>(r);
		}
		else if constexpr (std::is_same_v<LType, double> || std::is_same_v<RType, double>)
		{
			return static_cast<double>(l) / static_cast<double>(r);
		}
		else
		{
			throw std::invalid_argument("Unsupported combination");
		}
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
			throw std::invalid_argument("Cannot mod bool values");
		}
		else if constexpr (std::is_same_v<LType, HeapObjectPtr> || std::is_same_v<RType, HeapObjectPtr>)
		{
			if (IsString(l) || IsString(r))
			{
				throw std::invalid_argument("Cannot mod strings values");
			}
		}
		else if constexpr (std::is_integral_v<LType> && std::is_integral_v<RType>)
		{
			using CommonInt = std::conditional_t<std::is_same_v<LType, int64_t> || std::is_same_v<RType, int64_t>, int64_t, char>;
			return static_cast<CommonInt>(l) % static_cast<CommonInt>(r);
		}
		else if constexpr (std::is_same_v<LType, double> || std::is_same_v<RType, double>)
		{
			throw std::invalid_argument("Cannot mod non-integers");
		}
		else
		{
			throw std::invalid_argument("Unsupported combination");
		}
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
			throw std::invalid_argument("Cannot apply unary minus to bool");
		}
		else if constexpr (std::is_same_v<Type, HeapObjectPtr>)
		{
			if (IsString(v))
			{
				throw std::invalid_argument("Cannot apply unary minus to string values");
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
		else
		{
			throw std::invalid_argument("Unsupported type for unary minus");
		}
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
