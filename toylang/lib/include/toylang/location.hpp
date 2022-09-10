#pragma once
#include <string_view>

namespace toylang {
struct CharSpan {
	std::size_t first{};
	std::size_t last{};

	constexpr std::string_view view(std::string_view full_text) const {
		if (first > full_text.size()) { return {}; }
		auto const size = last - first;
		return full_text.substr(first, size > full_text.size() ? std::string_view::npos : size);
	}
};

///
/// \brief Source location
///
struct Location {
	std::string_view full_text{};
	std::uint32_t line{1};
	CharSpan char_span{};
};
} // namespace toylang
