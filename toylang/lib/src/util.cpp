#include <toylang/util.hpp>
#include <toylang/value.hpp>
#include <cstdio>
#include <fstream>

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

void util::print(char const* str) { std::printf("%s", unescape(str).c_str()); }

std::string util::read_file(char const* path) {
	auto file = std::ifstream(path, std::ios::ate);
	if (!file) { return {}; }
	auto const size = file.tellg();
	file.seekg({});
	auto ret = std::string{};
	ret.resize(static_cast<std::size_t>(size));
	file.read(ret.data(), size);
	return ret;
}

bool util::write_file(char const* path, std::string_view text) {
	auto file = std::ofstream(path);
	if (!file) { return false; }
	file.write(text.data(), static_cast<std::streamsize>(text.size()));
	return true;
}
} // namespace toylang
