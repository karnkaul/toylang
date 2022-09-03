#include <toylang/parser.hpp>

namespace toylang {
namespace {
struct ParseError {};
} // namespace

Parser::Parser(std::string_view text, ErrorReporter const* reporter) : m_scanner{text, reporter} { advance(); }

UExpr Parser::parse() {
	if (!m_current) { return {}; }
	try {
		return expression();
	} catch (ParseError const& err) { return {}; }
}

UExpr Parser::equality() {
	auto ret = comparison();
	while (advance_if(TokenType::eBangEq, TokenType::eEqEq)) {
		auto op = prev();
		auto right = comparison();
		ret = std::make_unique<BinaryExpr>(std::move(ret), op, std::move(right));
	}
	return ret;
}

UExpr Parser::comparison() {
	auto ret = term();
	while (advance_if(TokenType::eGt, TokenType::eGe, TokenType::eLt, TokenType::eLe)) {
		auto op = prev();
		auto right = comparison();
		ret = std::make_unique<BinaryExpr>(std::move(ret), op, std::move(right));
	}
	return ret;
}

UExpr Parser::term() {
	auto ret = factor();
	while (advance_if(TokenType::eMinus, TokenType::ePlus)) {
		auto op = prev();
		auto right = factor();
		ret = std::make_unique<BinaryExpr>(std::move(ret), op, std::move(right));
	}
	return ret;
}

UExpr Parser::factor() {
	auto ret = unary();
	while (advance_if(TokenType::eSlash, TokenType::eStar)) {
		auto op = prev();
		auto right = unary();
		ret = std::make_unique<BinaryExpr>(std::move(ret), op, std::move(right));
	}
	return ret;
}

UExpr Parser::unary() {
	if (advance_if(TokenType::eBang, TokenType::eMinus)) {
		auto op = prev();
		auto right = unary();
		return std::make_unique<UnaryExpr>(op, std::move(right));
	}
	return primary();
}

UExpr Parser::primary() {
	if (advance_if(TokenType::eFalse)) { return std::make_unique<LiteralExpr>(Bool{false}); }
	if (advance_if(TokenType::eTrue)) { return std::make_unique<LiteralExpr>(Bool{true}); }
	if (advance_if(TokenType::eNull)) { return std::make_unique<LiteralExpr>(nullptr); }
	if (advance_if(TokenType::eNumber, TokenType::eString)) { return std::make_unique<LiteralExpr>(prev().lexeme); }
	if (advance_if(TokenType::eParenL)) {
		auto expr = expression();
		consume(TokenType::eParenR, ")");
		return std::make_unique<GroupExpr>(std::move(expr));
	}
	m_scanner.reporter()->unexpected_token(m_scanner.make_error_context(m_current.location));
	throw ParseError{};
}

Token const& Parser::advance() {
	m_previous = std::move(m_current);
	m_current = m_scanner.next_token();
	return peek();
}

Token const& Parser::consume(TokenType type, std::string_view expect) noexcept(false) {
	if (check(type)) { return advance(); }
	m_scanner.reporter()->unexpected_token(m_scanner.make_error_context(m_current.location), expect);
	throw ParseError{};
}

template <std::same_as<TokenType>... Types>
bool Parser::advance_if(Types... any_of) {
	if ((... || (peek().type == any_of))) {
		advance();
		return true;
	}
	return false;
}
} // namespace toylang
