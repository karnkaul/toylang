#pragma once
#include <toylang/util/buffer.hpp>
#include <span>
#include <string>

namespace toylang {
class Value;

template <typename T>
concept Appendable = requires(std::string& s, T t) {
	s += t;
};

namespace util {
std::string unescape(std::string_view str);
std::string concat(std::span<Value const> values, std::string_view delim = " ");
void print_unescaped(std::string_view str);

template <Appendable... T>
void append(std::string& out, T const&... t) {
	((out += t), ...);
}
} // namespace util
} // namespace toylang
