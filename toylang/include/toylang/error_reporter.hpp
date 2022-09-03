#pragma once
#include <toylang/location.hpp>
#include <string_view>

namespace toylang {
struct ErrorContext {
	struct Marker {
		std::size_t first{};
		std::size_t extent{};
	};

	std::string_view line{};
	Marker marker{};
	Location location{};

	constexpr std::string_view marked() const {
		if (marker.first >= line.size()) { return "[eof]"; }
		return line.substr(marker.first, marker.extent);
	}
};

std::string make_marker(std::string_view const text, Location const& loc);

struct ErrorReporter {
	virtual ~ErrorReporter() = default;

	virtual void unexpected_token(ErrorContext const& context, std::string_view expected = {}) const = 0;
	virtual void unterminated_string(ErrorContext const& context) const = 0;
};

struct StdErrReporter : ErrorReporter {
	void print(std::string_view message) const;

	void unexpected_token(ErrorContext const& context, std::string_view expected) const override;
	void unterminated_string(ErrorContext const& context) const override;
};

inline auto const stderr_reporter = StdErrReporter{};

inline constexpr ErrorReporter const* get_reporter(ErrorReporter const* in) { return in ? in : &stderr_reporter; }
} // namespace toylang
