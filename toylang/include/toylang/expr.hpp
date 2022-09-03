#pragma once
#include <toylang/token.hpp>
#include <toylang/trivial.hpp>
#include <memory>

namespace toylang {
struct Expr {
	struct Visitor;
	struct Printer;

	virtual ~Expr() = default;
	virtual void accept(Visitor& out) const = 0;
};

using UExpr = std::unique_ptr<Expr>;

struct LiteralExpr : Expr {
	Trivial value{};

	LiteralExpr(Trivial value) : value{std::move(value)} {}
	void accept(Visitor& out) const override;
};

struct GroupExpr : Expr {
	UExpr expr{};

	GroupExpr(UExpr expr) : expr{std::move(expr)} {}
	void accept(Visitor& out) const override;
};

struct UnaryExpr : Expr {
	Token op{};
	UExpr right{};

	UnaryExpr(Token op, UExpr right) : op{op}, right{std::move(right)} {}
	void accept(Visitor& out) const override;
};

struct BinaryExpr : Expr {
	UExpr left{};
	Token op{};
	UExpr right{};

	BinaryExpr(UExpr left, Token op, UExpr right) : left{std::move(left)}, op{op}, right{std::move(right)} {}
	void accept(Visitor& out) const override;
};

struct Expr::Visitor {
	virtual void visit(LiteralExpr const&) = 0;
	virtual void visit(GroupExpr const&) = 0;
	virtual void visit(UnaryExpr const&) = 0;
	virtual void visit(BinaryExpr const&) = 0;
};

struct Expr::Printer : Visitor {
	std::string& out;

	Printer(std::string& out) : out(out) {}

	void visit(LiteralExpr const&) override;
	void visit(GroupExpr const&) override;
	void visit(UnaryExpr const&) override;
	void visit(BinaryExpr const&) override;
};

std::string to_string(Expr const& expr);
} // namespace toylang
