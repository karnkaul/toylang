#include <toylang/parser.hpp>
#include <toylang/util.hpp>
#include <toylang/util/notifier.hpp>
#include <toylang/value.hpp>

namespace toylang {
namespace {
struct ParseError {};

constexpr bool is_reserved(std::string_view identifier) {
	assert(!identifier.empty());
	return identifier[0] == '_';
}

std::string args_overflow_error_str(std::string kind, std::size_t arity) {
	auto ret = std::string{};
	ret.reserve(128);
	util::append(ret, "Too many ", kind, ": ", std::to_string(arity), " (max: ", std::to_string(max_args_v), ")");
	return ret;
};
} // namespace

struct Parser::Scope {
	Parser& in;

	Scope(Parser& in) : in(in) { in.m_flags |= eScoped; }
	~Scope() noexcept { in.m_flags &= ~eScoped; }
};

Parser::Parser(Source source, util::Notifier* notifier) : m_notifier{notifier}, m_scanner{source, m_notifier} { advance(); }
Parser::Parser(Quiet, Source source) : m_scanner{source} { advance(); }

bool Parser::is_expression(std::string_view text) {
	auto parser = Parser{Quiet{}, {.text = text}};
	if (parser.parse_expr() && parser.at_end()) { return true; }
	return false;
}

StmtImport Parser::parse_import() {
	if (advance_if(TokenType::eImport)) {
		try {
			auto path = consume(TokenType::eString);
			consume(TokenType::eSemicolon);
			return StmtImport{std::move(path)};
		} catch (ParseError const&) {}
	}
	return {};
}

UExpr Parser::parse_expr() {
	try {
		return equality();
	} catch (ParseError const&) {}
	return {};
}

UStmt Parser::parse_stmt() {
	while (!at_end()) {
		try {
			return declaration();
		} catch (ParseError const&) { synchronize(); }
	}
	return {};
}

UExpr Parser::expression() { return assignment(); }

UExpr Parser::assignment() {
	auto expr = expr_or();
	if (advance_if(TokenType::eEq)) {
		auto const token = prev();
		auto value = assignment();
		if (auto var = dynamic_cast<ExprVar*>(expr.get())) {
			return std::make_unique<ExprAssign>(var->name, std::move(value));
		} else if (auto get = dynamic_cast<ExprGet*>(expr.get())) {
			return std::make_unique<ExprSet>(std::move(get->obj), std::move(get->name), std::move(value));
		}
		if (m_notifier) { (*m_notifier)(m_scanner.make_diagnostic(token, "Invalid assignment target")); }
	}
	return expr;
}

UExpr Parser::expr_or() {
	auto ret = expr_and();
	while (advance_if(TokenType::eOr)) {
		auto op = prev();
		auto rhs = expr_and();
		ret = std::make_unique<ExprLogical>(std::move(ret), op, std::move(rhs));
	}
	return ret;
}

UExpr Parser::expr_and() {
	auto ret = equality();
	while (advance_if(TokenType::eAnd)) {
		auto op = prev();
		auto rhs = equality();
		ret = std::make_unique<ExprLogical>(std::move(ret), op, std::move(rhs));
	}
	return ret;
}

UExpr Parser::equality() {
	auto ret = comparison();
	while (advance_if(TokenType::eBangEq, TokenType::eEqEq)) {
		auto op = prev();
		auto rhs = comparison();
		ret = std::make_unique<ExprBinary>(std::move(ret), op, std::move(rhs));
	}
	return ret;
}

UExpr Parser::comparison() {
	auto ret = term();
	while (advance_if(TokenType::eGt, TokenType::eGe, TokenType::eLt, TokenType::eLe)) {
		auto op = prev();
		auto rhs = term();
		ret = std::make_unique<ExprBinary>(std::move(ret), op, std::move(rhs));
	}
	return ret;
}

UExpr Parser::term() {
	auto ret = factor();
	while (advance_if(TokenType::eMinus, TokenType::ePlus)) {
		auto op = prev();
		auto rhs = factor();
		ret = std::make_unique<ExprBinary>(std::move(ret), op, std::move(rhs));
	}
	return ret;
}

UExpr Parser::factor() {
	auto ret = unary();
	while (advance_if(TokenType::eSlash, TokenType::eStar)) {
		auto op = prev();
		auto rhs = unary();
		ret = std::make_unique<ExprBinary>(std::move(ret), op, std::move(rhs));
	}
	return ret;
}

UExpr Parser::unary() {
	if (advance_if(TokenType::eBang, TokenType::eMinus)) {
		auto op = prev();
		auto rhs = unary();
		return std::make_unique<ExprUnary>(op, std::move(rhs));
	}
	return invoke();
}

UExpr Parser::invoke() {
	auto ret = primary();
	while (true) {
		if (advance_if(TokenType::eParenL)) {
			ret = finish_invoke(std::move(ret));
			continue;
		}
		if (advance_if(TokenType::eDot)) {
			auto name = consume(TokenType::eIdentifier);
			ret = std::make_unique<ExprGet>(std::move(ret), std::move(name));
			continue;
		}
		break;
	}
	return ret;
}

UExpr Parser::primary() {
	if (m_current.type == TokenType::eEof) { throw ParseError{}; }
	if (advance_if(TokenType::eFalse)) { return std::make_unique<ExprLiteral>(Bool{false}, prev()); }
	if (advance_if(TokenType::eTrue)) { return std::make_unique<ExprLiteral>(Bool{true}, prev()); }
	if (advance_if(TokenType::eNull)) { return std::make_unique<ExprLiteral>(nullptr, prev()); }
	if (advance_if(TokenType::eNumber)) { return std::make_unique<ExprLiteral>(std::atof(std::string{prev().lexeme}.c_str()), prev()); }
	if (advance_if(TokenType::eString)) { return std::make_unique<ExprLiteral>(prev().lexeme, prev()); }
	if (advance_if(TokenType::eIdentifier)) { return std::make_unique<ExprVar>(prev()); }
	if (advance_if(TokenType::eParenL)) {
		if (at_end()) { unwind(TokenType::eParenR, "Unexpected EOF"); }
		auto expr = expression();
		consume(TokenType::eParenR);
		return std::make_unique<ExprGroup>(std::move(expr));
	}
	if (m_notifier) { (*m_notifier)(m_scanner.make_diagnostic(m_current, "Unexpected token")); }
	throw ParseError{};
}

UStmt Parser::declaration() {
	if (advance_if(TokenType::eStruct)) { return decl_struct(); }
	if (advance_if(TokenType::eFn)) { return decl_fn(); }
	if (advance_if(TokenType::eVar)) { return decl_var(); }
	return statement();
}

UPtr<StmtFn> Parser::decl_fn() {
	auto name = consume(TokenType::eIdentifier);
	auto params = StmtFn::Params{};
	auto arity = std::size_t{};
	consume(TokenType::eParenL);
	if (!check(TokenType::eParenR)) {
		do {
			if (params.has_space()) { params.add(consume(TokenType::eIdentifier)); }
			++arity;
		} while (advance_if(TokenType::eComma));
	}
	consume(TokenType::eParenR);
	consume(TokenType::eBraceL);
	if ((m_flags & eScoped) == eScoped) { unwind(TokenType::eEof, "fn only permitted in global scope", name); }
	auto block = make_block();
	if (arity >= max_args_v) { unwind(TokenType::eParenR, args_overflow_error_str("parameters", arity)); }
	return std::make_unique<StmtFn>(name, std::move(params), std::move(block));
}

UPtr<StmtVar> Parser::decl_var() {
	auto name = consume(TokenType::eIdentifier);
	if (is_reserved(name.lexeme)) {
		if (m_notifier) { (*m_notifier)(m_scanner.make_diagnostic(name, "Identifiers starting with _ are reserved")); }
		throw ParseError{};
	}
	auto initializer = UExpr{};
	if (advance_if(TokenType::eEq)) { initializer = expression(); }
	if (!initializer) { initializer = std::make_unique<ExprLiteral>(nullptr, peek()); }
	consume(TokenType::eSemicolon);
	return std::make_unique<StmtVar>(std::move(name), std::move(initializer));
}

UPtr<StmtStruct> Parser::decl_struct() {
	auto name = consume(TokenType::eIdentifier);
	auto vars = std::vector<UPtr<StmtVar>>{};
	consume(TokenType::eBraceL);
	if (!advance_if(TokenType::eBraceR)) {
		do {
			if (advance_if(TokenType::eFn)) {
				auto token = prev();
				decl_fn();
				if (m_notifier) { (*m_notifier)(m_scanner.make_diagnostic(token, "fn not allowed in structs")); }
				continue;
			}
			if (!advance_if(TokenType::eVar)) { unwind(TokenType::eEof, "Invalid statement in struct declaration"); }
			vars.push_back(decl_var());
		} while (!check(TokenType::eBraceR));
		consume(TokenType::eBraceR);
	}
	return std::make_unique<StmtStruct>(std::move(name), std::move(vars));
}

UStmt Parser::statement() {
	if (advance_if(TokenType::eFor)) { return stmt_for(); }
	if (advance_if(TokenType::eIf)) { return stmt_if(); }
	if (advance_if(TokenType::eWhile)) { return stmt_while(); }
	if (advance_if(TokenType::eBraceL)) { return stmt_block(); }
	if (advance_if(TokenType::eBreak)) { return stmt_break(); }
	if (advance_if(TokenType::eReturn)) { return stmt_return(); }
	return stmt_expr();
}

UPtr<StmtWhile> Parser::stmt_while() {
	consume(TokenType::eParenL);
	auto condition = expression();
	consume(TokenType::eParenR);
	if (!advance_if(TokenType::eBraceL)) { unwind(TokenType::eBraceL, "Block required after while"); }
	auto body = stmt_block();
	return std::make_unique<StmtWhile>(std::move(condition), std::move(body));
}

UPtr<StmtBlock> Parser::stmt_for() {
	consume(TokenType::eParenL);
	auto outer_body = std::vector<UStmt>{};
	if (!advance_if(TokenType::eSemicolon)) {
		if (advance_if(TokenType::eVar)) {
			outer_body.push_back(decl_var());
		} else {
			outer_body.push_back(stmt_expr());
		}
	}
	auto condition = UExpr{};
	if (!check(TokenType::eSemicolon)) { condition = expression(); }
	consume(TokenType::eSemicolon);
	if (!condition) { condition = std::make_unique<ExprLiteral>(Bool{true}, Token{}); }
	auto increment = UExpr{};
	if (!check(TokenType::eParenR)) { increment = expression(); }
	consume(TokenType::eParenR);
	auto inner_body = std::vector<UStmt>{};
	inner_body.push_back(statement());
	inner_body.push_back(std::make_unique<StmtExpr>(std::move(increment)));
	auto loop = std::make_unique<StmtWhile>(std::move(condition), std::make_unique<StmtBlock>(std::move(inner_body)));
	outer_body.push_back(std::move(loop));
	return std::make_unique<StmtBlock>(std::move(outer_body));
}

UPtr<StmtBlock> Parser::stmt_block() { return std::make_unique<StmtBlock>(make_block()); }

UPtr<StmtExpr> Parser::stmt_expr() {
	auto expr = expression();
	consume(TokenType::eSemicolon);
	return std::make_unique<StmtExpr>(std::move(expr));
}

UPtr<StmtBreak> Parser::stmt_break() {
	auto token = prev();
	consume(TokenType::eSemicolon);
	return std::make_unique<StmtBreak>(StmtBreak::Break{token});
}

UPtr<StmtReturn> Parser::stmt_return() {
	auto token = prev();
	auto ret = UExpr{};
	if (!check(TokenType::eSemicolon)) { ret = expression(); }
	consume(TokenType::eSemicolon);
	return std::make_unique<StmtReturn>(StmtReturn::Return{token}, std::move(ret));
}

UPtr<StmtIf> Parser::stmt_if() {
	consume(TokenType::eParenL);
	auto condition = expression();
	consume(TokenType::eParenR);
	if (!advance_if(TokenType::eBraceL)) { unwind(TokenType::eBraceL, "Block required after if"); }
	auto on = stmt_block();
	auto off = UStmt{};
	if (advance_if(TokenType::eElse)) {
		if (!advance_if(TokenType::eBraceL)) { unwind(TokenType::eBraceL, "Block required after else"); }
		off = stmt_block();
	}
	return std::make_unique<StmtIf>(std::move(condition), std::move(on), std::move(off));
}

std::vector<UStmt> Parser::make_block() {
	auto scope = Scope{*this};
	auto ret = std::vector<UStmt>{};
	while (!at_end() && !check(TokenType::eBraceR)) { ret.push_back(declaration()); }
	consume(TokenType::eBraceR);
	return ret;
}

UExpr Parser::finish_invoke(UExpr&& callee) {
	auto args = ExprInvoke::Args{};
	auto arity = std::size_t{};
	if (!check(TokenType::eParenR)) {
		do {
			auto expr = expression();
			if (args.has_space()) { args.add(std::move(expr)); }
			++arity;
		} while (advance_if(TokenType::eComma));
	}
	auto const paren_r = consume(TokenType::eParenR);
	if (arity > max_args_v && m_notifier) { (*m_notifier)(m_scanner.make_diagnostic(paren_r, args_overflow_error_str("arguments", arity))); }
	return std::make_unique<ExprInvoke>(std::move(callee), std::move(paren_r), std::move(args));
}

Token const& Parser::advance() {
	m_previous = std::move(m_current);
	m_current = m_scanner.next_token();
	return peek();
}

Token const& Parser::consume(TokenType type) noexcept(false) {
	if (check(type)) {
		advance();
		return prev();
	}
	unwind(type, "Unexpected token");
	return prev();
}

void Parser::unwind(TokenType expected, std::string_view message, Token at) noexcept(false) {
	if (!at) { at = m_current; }
	if (m_notifier) { (*m_notifier)(m_scanner.make_diagnostic(at, message, expected)); }
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

void Parser::synchronize() {
	advance();
	while (!at_end() && prev().type != TokenType::eSemicolon) { advance(); }
}
} // namespace toylang
