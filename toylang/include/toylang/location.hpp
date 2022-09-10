#pragma once
#include <cstdint>

namespace toylang {
struct Location {
	std::uint32_t line{1};
	std::size_t first{};
	std::size_t last{};

	bool operator==(Location const& rhs) const = default;
};
} // namespace toylang
