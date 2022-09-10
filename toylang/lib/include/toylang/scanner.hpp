#pragma once
#include <toylang/diagnostic.hpp>
#include <toylang/source.hpp>
#include <toylang/token.hpp>
#include <cassert>

namespace toylang {
template <typename TNotifier>
class Scanner {
  public:
	Scanner() = default;
	constexpr Scanner(Source source, TNotifier* notifier = {}) : m_notifier{notifier} {
		m_current.filename = source.filename;
		m_current.full_text = source.text;
	}

	constexpr Token next_token() {
		auto ret = scan_token();
		m_current.char_span.first = m_current.char_span.last;
		return ret;
	}

	static constexpr Diagnostic make_diagnostic(Token const& token, std::string_view message, TokenType expected = TokenType::eEof) {
		return Diagnostic{token, expected, message, Diagnostic::Type::eSyntaxError};
	}

  private:
	static constexpr bool in_range(char const ch, char const a, char const b) { return a <= ch && ch <= b; }
	static constexpr bool is_digit(char const ch) { return in_range(ch, '0', '9'); }
	static constexpr bool is_whitespace(char const ch) { return ch == ' ' || ch == '\t'; }
	static constexpr bool is_newline(char const ch) { return ch == '\n'; }
	static constexpr bool is_alpha(char const ch) { return in_range(ch, 'A', 'Z') || in_range(ch, 'a', 'z'); }
	static constexpr bool starts_identifier(char const ch) { return is_alpha(ch) || ch == '_'; }
	static constexpr bool match_identifier(char const c) { return starts_identifier(c) || is_digit(c); }

	constexpr bool at_end() const { return m_current.char_span.last >= m_current.full_text.size(); }
	constexpr char peek() const { return m_current.char_span.last >= m_current.full_text.size() ? '\0' : m_current.full_text[m_current.char_span.last]; }
	constexpr char peek_next() const {
		return m_current.char_span.last + 1 >= m_current.full_text.size() ? '\0' : m_current.full_text[m_current.char_span.last + 1];
	}

	constexpr std::size_t start_of_line(std::size_t start) const {
		for (std::size_t i = start; i + 1 > 0; --i) {
			if (m_current.full_text[i] == '\n') { return i; }
		}
		return 0;
	}

	constexpr Token make_token(TokenType type, std::string_view lexeme, Location location = {}) const {
		return Token{
			.lexeme = lexeme,
			.location = location,
			.type = type,
		};
	}

	constexpr Token make_token(TokenType type) const {
		return make_token(type, m_current.full_text.substr(m_current.char_span.first, m_current.char_span.last - m_current.char_span.first), m_current);
	}

	constexpr bool munch(Token& out, std::string_view const keyword, TokenType type) {
		if (m_current.full_text.size() < m_current.char_span.first + keyword.size()) { return false; }
		if (m_current.full_text.substr(m_current.char_span.first, keyword.size()) != keyword) { return false; }
		m_current.char_span.last = m_current.char_span.first + keyword.size();
		out = make_token(type);
		return true;
	}

	constexpr bool make_string(Token& out) {
		while (peek() != '\"' && !at_end()) {
			if (peek() == '\n') { ++m_current.line; }
			advance();
		}
		if (at_end()) {
			if (m_notifier) { (*m_notifier)(make_diagnostic(make_token(TokenType::eString), "Unterminated string")); }
			return false;
		}
		// closing "
		advance();
		out = {
			.lexeme = m_current.full_text.substr(m_current.char_span.first + 1, m_current.char_span.last - m_current.char_span.first - 2),
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
		for (TokenType type = keyword_range_v.first; type < keyword_range_v.second; type = increment(type)) {
			if (munch(ret, token_string(type), type)) { return ret; }
		}
		while (!at_end() && match_identifier(peek())) { advance(); }
		return make_token(TokenType::eIdentifier);
	}

	constexpr bool try_single(Token& out, char const c) {
		for (TokenType type = single_range_v.first; type < single_range_v.second; type = increment(type)) {
			if (token_string(type)[0] == c) {
				out = make_token(type);
				return true;
			}
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
			++m_current.char_span.first;
			return true;
		}
		if (is_newline(c)) {
			++m_current.char_span.first;
			++m_current.line;
			return true;
		}
		return false;
	}

	constexpr char advance() {
		assert(!at_end());
		return m_current.full_text[m_current.char_span.last++];
	}

	constexpr bool advance_if(char expected) {
		assert(expected != '\0');
		if (peek() != expected) { return false; }
		++m_current.char_span.last;
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
			if (starts_identifier(c)) { return make_identifier(); }
			if (m_notifier) { (*m_notifier)(make_diagnostic(make_token(TokenType::eString), "Unexpected token")); }
			++m_current.char_span.first;
		}
		return make_token(TokenType::eEof);
	}

	Location m_current{};
	TNotifier* m_notifier{};
};
} // namespace toylang
