#include <toylang/util.hpp>
#include <toylang/util/reporter.hpp>
#include <cassert>
#include <cstdio>

namespace toylang::util {
namespace {
struct Marker {
	std::size_t first{};
	std::size_t extent{};
};

constexpr std::string_view type_str_v[] = {"Runtime Error", "Syntax Error", "Internal Error", "Warning"};
constexpr std::string_view type_string(Diagnostic::Type const type) { return detail::at(type_str_v, type, std::string_view{"Unknown Error"}); }

struct PrintData {
	std::string_view filename{};
	std::string_view line{};
	Marker marker{};

	constexpr std::string_view marked() const {
		if (marker.first >= line.size()) { return "[eof]"; }
		return line.substr(marker.first, marker.extent);
	}
};

constexpr std::size_t start_of_line(std::string_view const text, std::size_t const start) {
	if (text.empty()) { return 0; }
	for (std::size_t i = start; i + 1 > 0; --i) {
		if (text[i] == '\n') { return i + 1; }
	}
	return 0;
}

constexpr std::string_view make_line(std::string_view const text, std::size_t const start) {
	auto ret = text.substr(start);
	return ret.substr(0, ret.find('\n'));
}

constexpr PrintData make_data(Token const& token) {
	auto const start = start_of_line(token.location.full_text, token.location.char_span.first);
	return {
		.filename = token.location.filename,
		.line = make_line(token.location.full_text, start),
		.marker = {token.location.char_span.first - start, token.location.char_span.last - token.location.char_span.first},
	};
}

std::string make_marker(PrintData const& data, Location const& loc, char const mark) {
	auto ret = std::string{};
	auto buffer = std::string{};
	auto const line_str = std::to_string(loc.line);
	util::append(buffer, "  ", line_str, " | ");
	util::append(ret, buffer, data.line, '\n');
	buffer.clear();
	util::append(buffer, "  ", std::string(line_str.size(), ' '), " | ");
	for (std::size_t i = 0; i < data.marker.first; ++i) {
		char const c = data.line[i];
		if (std::isspace(static_cast<unsigned char>(c))) {
			util::append(buffer, c);
		} else {
			util::append(buffer, ' ');
		}
	}
	util::append(ret, buffer, std::string(std::max(data.marker.extent, 1UL), mark), '\n');
	return ret;
}

std::string format(PrintData const& data, Diagnostic const& diag, char quote, char mark) {
	auto ret = std::string{type_string(diag.type)};
	util::append(ret, ": ", diag.message);
	util::append(ret, "  ", quote, data.marked(), quote, '\n', make_marker(data, diag.token.location, mark), '\n');
	if (!data.filename.empty()) { util::append(ret, data.filename, '\n'); }
	if (diag.expected != TokenType::eEof) { util::append(ret, "Expected: ", quote, token_string(diag.expected), quote, "\n"); }
	return ret;
}
} // namespace

void Reporter::on_notify(Diagnostic const& diag) {
	auto fptr = stdout;
	if (diag.is_error()) {
		m_data.error = true;
		fptr = stderr;
	}
	auto const ctx = make_data(diag.token);
	std::fprintf(fptr, "%s\n", format(ctx, diag, quote, mark).c_str());
}
} // namespace toylang::util
