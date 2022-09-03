#pragma once
#include <toylang/error_reporter.hpp>
#include <toylang/token.hpp>
#include <cassert>

namespace toylang {
namespace detail {
template <typename Enum, typename Cont>
constexpr auto at(Cont const& ts, Enum const e, decltype(ts[0]) const& fallback = {}) {
	auto const index = static_cast<std::size_t>(e);
	if (index >= std::size(ts)) { return fallback; }
	return ts[index];
}
} // namespace detail

inline constexpr bool in_range(char const ch, char const a, char const b) { return a <= ch && ch <= b; }
inline constexpr bool is_digit(char const ch) { return in_range(ch, '0', '9'); }
inline constexpr bool is_whitespace(char const ch) { return ch == ' ' || ch == '\t'; }
inline constexpr bool is_newline(char const ch) { return ch == '\n'; }
inline constexpr bool is_alpha(char const ch) { return in_range(ch, 'A', 'Z') || in_range(ch, 'a', 'z') || ch == '_' || ch == ':'; }

class Scanner {
  public:
	Scanner() = default;
	constexpr Scanner(std::string_view text, ErrorReporter const* reporter = {}) : m_text(text), m_reporter(get_reporter(reporter)) {}

	constexpr Token next_token() {
		auto ret = scan_token();
		m_current.first = m_current.last;
		return ret;
	}

	constexpr ErrorReporter const* reporter() const { return m_reporter; }

	constexpr std::string_view make_line(std::size_t start) const {
		auto ret = m_text.substr(start);
		return ret.substr(0, ret.find('\n'));
	}

	constexpr ErrorContext make_error_context(Location const& loc) const {
		auto const start = start_of_line(loc.first);
		return {
			.line = make_line(start),
			.marker = {loc.first - start, loc.last - loc.first},
			.location = m_current,
		};
	}

  private:
	constexpr bool at_end() const { return m_current.last >= m_text.size(); }
	constexpr char peek() const { return m_current.last >= m_text.size() ? '\0' : m_text[m_current.last]; }
	constexpr char peek_next() const { return m_current.last + 1 >= m_text.size() ? '\0' : m_text[m_current.last + 1]; }

	constexpr std::size_t start_of_line(std::size_t start) const {
		for (std::size_t i = start; i + 1 > 0; --i) {
			if (m_text[i] == '\n') { return i; }
		}
		return 0;
	}

	constexpr Token make_token(TokenType type) const {
		return {
			.lexeme = m_text.substr(m_current.first, m_current.last - m_current.first),
			.location = m_current,
			.type = type,
		};
	}

	constexpr bool munch(Token& out, std::string_view const keyword, TokenType type) {
		if (m_text.size() < m_current.first + keyword.size()) { return false; }
		if (m_text.substr(m_current.first, keyword.size()) != keyword) { return false; }
		m_current.last = m_current.first + keyword.size();
		out = make_token(type);
		return true;
	}

	constexpr bool make_string(Token& out) {
		while (peek() != '\"' && !at_end()) {
			if (peek() == '\n') { ++m_current.line; }
			advance();
		}
		if (at_end()) {
			m_reporter->unterminated_string(make_error_context(m_current));
			return false;
		}
		// closing "
		advance();
		out = {
			.lexeme = m_text.substr(m_current.first + 1, m_current.last - m_current.first - 2),
			.location = m_current,
			.type = TokenType::eString,
		};
		return true;
	}

	constexpr Token make_number() {
		while (is_digit(peek())) { advance(); }
		if (peek() == '.' && is_digit(peek_next())) {
			// .
			advance();
			while (is_digit(peek())) { advance(); }
		}
		return make_token(TokenType::eNumber);
	}

	constexpr Token make_identifier() {
		auto ret = Token{};
		for (TokenType type = keyword_range_v.first; type <= keyword_range_v.second; type = increment(type)) {
			if (munch(ret, detail::at(token_str_v, type), type)) { return ret; }
		}
		while (!at_end() && is_alpha(peek())) { advance(); }
		return make_token(TokenType::eIdentifier);
	}

	constexpr bool try_single(Token& out, char const c) {
		switch (c) {
		case '+': out = make_token(TokenType::ePlus); return true;
		case '-': out = make_token(TokenType::eMinus); return true;
		case '*': out = make_token(TokenType::eStar); return true;
		case ',': out = make_token(TokenType::eComma); return true;
		case '.': out = make_token(TokenType::eDot); return true;
		case ';': out = make_token(TokenType::eSemicolon); return true;
		case '{': out = make_token(TokenType::eBraceL); return true;
		case '}': out = make_token(TokenType::eBraceR); return true;
		case '(': out = make_token(TokenType::eParenL); return true;
		case ')': out = make_token(TokenType::eParenR); return true;
		}
		return false;
	}

	constexpr bool try_double(Token& out, char const c) {
		switch (c) {
		case '!': out = make_token(advance_if('=') ? TokenType::eBangEq : TokenType::eBang); return true;
		case '=': out = make_token(advance_if('=') ? TokenType::eEqEq : TokenType::eEq); return true;
		case '<': out = make_token(advance_if('=') ? TokenType::eLe : TokenType::eLt); return true;
		case '>': out = make_token(advance_if('=') ? TokenType::eGe : TokenType::eGt); return true;
		}
		return false;
	}

	constexpr bool ignore(char const c) {
		if (is_whitespace(c)) {
			++m_current.first;
			return true;
		}
		if (is_newline(c)) {
			++m_current.line;
			return true;
		}
		return false;
	}

	constexpr char advance() {
		assert(!at_end());
		return m_text[m_current.last++];
	}

	constexpr bool advance_if(char expected) {
		assert(expected != '\0');
		if (peek() != expected) { return false; }
		++m_current.last;
		return true;
	}

	constexpr bool is_comment() {
		if (advance_if('/')) {
			while (peek() != '\n' && !at_end()) { advance(); }
			return true;
		}
		return false;
	}

	constexpr Token scan_token() {
		while (!at_end()) {
			char const c = advance();
			if (ignore(c)) { continue; }
			auto ret = Token{};
			if (c == '\"') {
				if (!make_string(ret)) { continue; }
				return ret;
			}
			if (try_single(ret, c)) { return ret; }
			if (try_double(ret, c)) { return ret; }
			if (c == '/') {
				if (is_comment()) {
					continue;
				} else {
					return make_token(TokenType::eSlash);
				}
			}
			if (is_digit(c)) { return make_number(); }
			if (is_alpha(c)) { return make_identifier(); }
			m_reporter->unexpected_token(make_error_context(m_current));
			++m_current.first;
		}
		return make_token(TokenType::eEof);
	}

	std::string_view m_text{};
	Location m_current{};
	ErrorReporter const* m_reporter{};
};
} // namespace toylang
