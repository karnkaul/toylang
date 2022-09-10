#include <toylang/util.hpp>
#include <toylang/value.hpp>
#include <cstdio>

namespace toylang {
std::string util::unescape(std::string_view in) {
	auto ret = std::string{};
	ret.reserve(in.size());
	for (std::size_t i = 0; i < in.size(); ++i) {
		if (in[i] == '\\' && i + 1 < in.size()) {
			switch (in[i + 1]) {
			case 'n': append(ret, '\n'); break;
			case 't': append(ret, '\t'); break;
			default: break;
			}
			++i;
			continue;
		}
		append(ret, in[i]);
	}
	return ret;
}

std::string util::concat(std::span<Value const> values, std::string_view delim) {
	auto ret = std::string{};
	bool first{true};
	for (auto const& value : values) {
		if (!first) { append(ret, delim); }
		first = false;
		append(ret, to_string(value));
	}
	return ret;
}

void util::print_unescaped(std::string_view str) { std::printf("%s", unescape(str).c_str()); }
} // namespace toylang
