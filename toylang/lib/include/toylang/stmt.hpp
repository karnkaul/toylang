#pragma once
#include <toylang/expr.hpp>
#include <vector>

namespace toylang {
///
/// \brief Base class for all statements
///
struct Stmt {
	struct Visitor;

	virtual ~Stmt() = default;
	virtual void accept(Visitor& out) const = 0;
};

using UStmt = UPtr<Stmt>;

struct StmtImport {
	Token path{};

	explicit operator bool() const { return static_cast<bool>(path); }
};

struct StmtExpr : Stmt {
	UExpr expr;

	StmtExpr(UExpr&& expr) : expr{std::move(expr)} {}
	void accept(Visitor& out) const override;
};

struct StmtVar : Stmt {
	Token name;
	UExpr initializer;

	StmtVar(Token name, UExpr&& initializer) : name{std::move(name)}, initializer{std::move(initializer)} {}
	void accept(Visitor& out) const override final;
};

struct StmtBlock : Stmt {
	std::vector<UStmt> statements;

	StmtBlock(std::vector<UStmt>&& statements) : statements{std::move(statements)} {}
	void accept(Visitor& out) const override final;
};

struct StmtIf : Stmt {
	UExpr condition;
	UStmt on;
	UStmt off;

	StmtIf(UExpr&& condition, UStmt&& on, UStmt&& off) : condition{std::move(condition)}, on{std::move(on)}, off{std::move(off)} {}
	void accept(Visitor& out) const override final;
};

struct StmtWhile : Stmt {
	UExpr condition;
	UStmt body;

	StmtWhile(UExpr&& condition, UStmt&& body) : condition(std::move(condition)), body(std::move(body)) {}
	void accept(Visitor& out) const override final;
};

struct StmtBreak : Stmt {
	struct Break {
		Token token{};
	};

	Break brk;

	StmtBreak(Break brk) : brk{std::move(brk)} {}
	void accept(Visitor& out) const override final;
};

struct StmtFn : Stmt {
	using Params = ArgsArray<Token>;

	Token name;
	Params params;
	std::vector<UStmt> body;

	StmtFn(Token name, Params&& params, std::vector<UStmt>&& body) : name{std::move(name)}, params{std::move(params)}, body{std::move(body)} {}
	void accept(Visitor& out) const override final;
};

struct StmtReturn : Stmt {
	struct Return {
		Token token{};
	};
	Return token;
	UExpr ret{};

	StmtReturn(Return token, UExpr&& ret) : token{std::move(token)}, ret{std::move(ret)} {}
	void accept(Visitor& out) const override final;
};

struct StmtStruct : Stmt {
	Token name;
	std::vector<UPtr<StmtVar>> vars{};

	StmtStruct(Token name, std::vector<UPtr<StmtVar>> vars) : name{std::move(name)}, vars{std::move(vars)} {}
	void accept(Visitor& out) const override final;
};

struct Stmt::Visitor {
	virtual void visit(StmtExpr const& stmt) = 0;
	virtual void visit(StmtVar const& stmt) = 0;
	virtual void visit(StmtBlock const& stmt) = 0;
	virtual void visit(StmtIf const& stmt) = 0;
	virtual void visit(StmtWhile const& stmt) = 0;
	virtual void visit(StmtBreak const& stmt) = 0;
	virtual void visit(StmtFn const& stmt) = 0;
	virtual void visit(StmtReturn const& stmt) = 0;
	virtual void visit(StmtStruct const& stmt) = 0;
};
} // namespace toylang
