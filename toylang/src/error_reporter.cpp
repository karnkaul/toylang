#include <toylang/error_reporter.hpp>
#include <toylang/token.hpp>
#include <cassert>
#include <iostream>

namespace toylang {
namespace {
template <typename... T>
void append(std::string& out, T const&... t) {
	((out += t), ...);
}

void write_loc(std::string& out, Location const& loc) { append(out, "At line ", std::to_string(loc.line), '\n'); }

std::string make_marker(std::string_view line, ErrorContext::Marker const& marker) {
	auto ret = std::string{};
	line = line.substr(0, line.find('\n'));
	append(ret, line, '\n', std::string(marker.first, ' '), std::string(std::max(marker.extent, std::size_t(1)), '~'), '\n');
	return ret;
}

std::string make_marker(ErrorContext const& context) {
	auto ret = std::string{};
	auto prefix = std::string{};
	auto const line_str = std::to_string(context.location.line);
	append(prefix, "  ", line_str, " | ");
	append(ret, prefix, context.line, '\n');
	prefix.clear();
	append(prefix, "  ", std::string(line_str.size(), ' '), " | ");
	append(ret, prefix, std::string(context.marker.first, ' '), std::string(std::max(context.marker.extent, 1UL), '~'), '\n');
	// append(ret, context.line, '\n', std::string(context.marker.first, ' '), std::string(std::max(context.marker.extent, std::size_t(1)), '~'), '\n');
	return ret;
}
} // namespace

void StdErrReporter::print(std::string_view message) const { std::cerr << message << '\n'; }

void StdErrReporter::unexpected_token(ErrorContext const& context, std::string_view const expected) const {
	auto out = std::string{"Unexpected token '"};
	append(out, context.marked(), "'\n", make_marker(context), '\n');
	if (!expected.empty()) { append(out, "Expected: '", expected, "'\n"); }
	print(out);
}

void StdErrReporter::unterminated_string(ErrorContext const& context) const {
	auto out = std::string{"Unterminated string: "};
	append(out, context.marked(), '\n', make_marker(context.line, context.marker));
	write_loc(out, context.location);
	print(out);
}
} // namespace toylang
