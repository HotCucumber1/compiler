#pragma once

#include <variant>
#include <string>
#include <memory>
#include <stdexcept>

// Forward declaration
// struct Function;
// using FunctionPtr = std::shared_ptr<Function>;
//
// class Value {
// private:
//     std::variant<long long, double, bool, std::string, FunctionPtr> m_data;
//
// public:
//     // Конструкторы
//     Value() : m_data(false) {}
//     Value(long long val) : m_data(val) {}
//     Value(double val) : m_data(val) {}
//     Value(bool val) : m_data(val) {}
//     Value(const std::string& val) : m_data(val) {}
//     Value(const FunctionPtr& func) : m_data(func) {}
//
//     // Проверка типов
//     bool isLongLong() const { return std::holds_alternative<long long>(m_data); }
//     bool isDouble() const { return std::holds_alternative<double>(m_data); }
//     bool isBool() const { return std::holds_alternative<bool>(m_data); }
//     bool isString() const { return std::holds_alternative<std::string>(m_data); }
//     bool isFunction() const { return std::holds_alternative<FunctionPtr>(m_data); }
//
//     // Получение значений
//     long long getLongLong() const { return std::get<long long>(m_data); }
//     double getDouble() const { return std::get<double>(m_data); }
//     bool getBool() const { return std::get<bool>(m_data); }
//     const std::string& getString() const { return std::get<std::string>(m_data); }
//     FunctionPtr getFunction() const { return std::get<FunctionPtr>(m_data); }
//
//     // Операторы
//     explicit operator bool() const {
//         return std::visit([](auto&& arg) -> bool {
//             using T = std::decay_t<decltype(arg)>;
//             if constexpr (std::is_same_v<T, long long>) {
//                 return arg != 0;
//             } else if constexpr (std::is_same_v<T, double>) {
//                 return arg != 0.0;
//             } else if constexpr (std::is_same_v<T, bool>) {
//                 return arg;
//             } else if constexpr (std::is_same_v<T, std::string>) {
//                 return !arg.empty();
//             } else if constexpr (std::is_same_v<T, FunctionPtr>) {
//                 return true;
//             }
//             return false;
//         }, m_data);
//     }
//
//     bool operator!() const {
//         return !static_cast<bool>(*this);
//     }
//
//     Value operator+(const Value& other) const;
//     Value operator-(const Value& other) const;
//     Value operator*(const Value& other) const;
//     Value operator/(const Value& other) const;
//     Value operator%(const Value& other) const;
//     Value operator-() const;
// };