#pragma once
#include <toylang/scanner.hpp>
#include <toylang/stmt.hpp>

namespace toylang {
namespace util {
class Notifier;
}

class Parser {
  public:
	struct Quiet {};

	Parser() = default;
	Parser(std::string_view text, util::Notifier* notifier = {});
	Parser(Quiet, std::string_view text);

	static bool is_expression(std::string_view text);

	StmtImport parse_import();
	UExpr parse_expr();
	UStmt parse_stmt();
	Token const& current() const { return m_current; }

  private:
	enum : std::uint32_t { eScoped = 1 << 0 };
	struct Scope;

	UExpr equality();
	UExpr assignment();
	UExpr expr_or();
	UExpr expr_and();
	UExpr expression();
	UExpr comparison();
	UExpr term();
	UExpr factor();
	UExpr unary();
	UExpr invoke();
	UExpr primary();

	UStmt declaration();
	UStmt statement();
	UPtr<StmtVar> decl_var();
	UPtr<StmtFn> decl_fn();
	UPtr<StmtStruct> decl_struct();
	UPtr<StmtWhile> stmt_while();
	UPtr<StmtBlock> stmt_for();
	UPtr<StmtBlock> stmt_block();
	UPtr<StmtExpr> stmt_expr();
	UPtr<StmtBreak> stmt_break();
	UPtr<StmtReturn> stmt_return();
	UPtr<StmtIf> stmt_if();

	std::vector<UStmt> make_block();
	UExpr finish_invoke(UExpr&& callee);

	bool at_end() const { return peek().type == TokenType::eEof; }
	bool check(TokenType type) const { return at_end() ? false : peek().type == type; }

	Token const& peek() const { return m_current; }
	Token const& prev() const { return m_previous; }
	Token const& advance();
	Token const& consume(TokenType type) noexcept(false);

	template <std::same_as<TokenType>... Types>
	bool advance_if(Types... any_of);

	void unwind(TokenType expected, std::string_view message, Token at = {}) noexcept(false);
	void synchronize();

	util::Notifier* m_notifier{};
	Scanner<util::Notifier> m_scanner{};
	Token m_previous{};
	Token m_current{};

	std::uint32_t m_flags{};
};
} // namespace toylang
