#pragma once
#include <toylang/expr.hpp>
#include <toylang/scanner.hpp>

namespace toylang {
class Parser {
  public:
	Parser(std::string_view text, ErrorReporter const* reporter = {});

	UExpr parse();

  private:
	UExpr equality();
	UExpr primary();
	UExpr expression() { return equality(); }
	UExpr comparison();
	UExpr term();
	UExpr factor();
	UExpr unary();

	bool at_end() const { return peek().type == TokenType::eEof; }
	bool check(TokenType type) const { return at_end() ? false : peek().type == type; }

	Token const& peek() const { return m_current; }
	Token const& prev() const { return m_previous; }
	Token const& advance();
	Token const& consume(TokenType type, std::string_view expect) noexcept(false);

	template <std::same_as<TokenType>... Types>
	bool advance_if(Types... any_of);

	Scanner m_scanner{};
	Token m_previous{};
	Token m_current{};
};
} // namespace toylang
