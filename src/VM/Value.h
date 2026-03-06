#pragma once
#include <iostream>
#include <stdexcept>
#include <variant>

using Value = std::variant<
	int64_t,
	double,
	bool,
	char>;

// OP_ADD
inline Value operator+(const Value& lhs, const Value& rhs)
{
	return std::visit([&]<typename T0, typename T1>(T0&& l, T1&& r) -> Value {
		using LType = std::decay_t<T0>;
		using RType = std::decay_t<T1>;

		if constexpr (std::is_same_v<LType, bool> || std::is_same_v<RType, bool>)
		{
			throw std::invalid_argument("Cannot add bool values");
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

// OP_NEG
inline Value operator-(const Value& value)
{
	return std::visit([]<typename T>(T&& v) -> Value {
		using Type = std::decay_t<T>;

		if constexpr (std::is_same_v<Type, bool>)
		{
			throw std::invalid_argument("Cannot apply unary minus to bool");
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
			std::cout << arg;
		},
			value);
		std::cout << std::endl;
	}
	catch (const std::bad_variant_access& e)
	{
		std::cout << "Error accessing variant: " << e.what() << std::endl;
	}
}