#pragma once
#include <string_view>

namespace toylang {
struct Source {
	std::string_view filename{};
	std::string_view text{};
};
} // namespace toylang
