#pragma once
#include <toylang/literal.hpp>
#include <toylang/token.hpp>
#include <memory>

namespace toylang {
template <typename Type>
using UPtr = std::unique_ptr<Type>;

struct Expr {
	struct Visitor;
	struct Printer;

	virtual ~Expr() = default;
	virtual void accept(Visitor& out) const = 0;
};

using UExpr = UPtr<Expr>;

inline constexpr std::size_t max_args_v{64};

template <typename Type>
struct ArgsArray {
	Type args[max_args_v];
	std::size_t arity{};

	bool has_space() const { return arity + 1 < max_args_v; }
	void add(Type&& t) { args[arity++] = std::move(t); }
	void add(Type const& t) { args[arity++] = t; }
};

struct ExprLiteral : Expr {
	Literal value;
	Token self;

	ExprLiteral(Literal value, Token self) : value{std::move(value)}, self(std::move(self)) {}
	void accept(Visitor& out) const override final;
};

struct ExprGroup : Expr {
	UExpr expr;

	ExprGroup(UExpr&& expr) : expr{std::move(expr)} {}
	void accept(Visitor& out) const override final;
};

struct ExprUnary : Expr {
	Token op;
	UExpr rhs;

	ExprUnary(Token op, UExpr&& rhs) : op{op}, rhs{std::move(rhs)} {}
	void accept(Visitor& out) const override final;
};

struct ExprBinary : Expr {
	UExpr lhs;
	Token op;
	UExpr rhs;

	ExprBinary(UExpr&& lhs, Token op, UExpr&& rhs) : lhs{std::move(lhs)}, op{op}, rhs{std::move(rhs)} {}
	void accept(Visitor& out) const override final;
};

struct ExprVar : Expr {
	Token name;

	ExprVar(Token name) : name{std::move(name)} {}
	void accept(Visitor& out) const override final;
};

struct ExprAssign : Expr {
	Token name;
	UExpr value;

	ExprAssign(Token name, UExpr&& value) : name{std::move(name)}, value{std::move(value)} {}
	void accept(Visitor& out) const override final;
};

struct ExprLogical : Expr {
	UExpr lhs;
	Token op;
	UExpr rhs;

	ExprLogical(UExpr&& lhs, Token op, UExpr&& rhs) : lhs{std::move(lhs)}, op{std::move(op)}, rhs{std::move(rhs)} {}
	void accept(Visitor& out) const override final;
};

struct ExprInvoke : Expr {
	using Args = ArgsArray<UExpr>;

	UExpr callee;
	Token paren_r;
	Args args;

	ExprInvoke(UExpr&& callee, Token token, Args&& args) : callee{std::move(callee)}, paren_r{std::move(token)}, args{std::move(args)} {}
	void accept(Visitor& out) const override final;
};

struct ExprGet : Expr {
	UExpr obj{};
	Token name{};

	ExprGet(UExpr&& obj, Token name) : obj{std::move(obj)}, name{std::move(name)} {}
	void accept(Visitor& out) const override final;
};

struct ExprSet : Expr {
	UExpr obj{};
	Token name{};
	UExpr value{};

	ExprSet(UExpr&& obj, Token name, UExpr&& value) : obj{std::move(obj)}, name{std::move(name)}, value{std::move(value)} {}
	void accept(Visitor& out) const final override;
};

struct Expr::Visitor {
	virtual void visit(ExprLiteral const&) = 0;
	virtual void visit(ExprGroup const&) = 0;
	virtual void visit(ExprUnary const&) = 0;
	virtual void visit(ExprBinary const&) = 0;
	virtual void visit(ExprVar const&) = 0;
	virtual void visit(ExprAssign const&) = 0;
	virtual void visit(ExprLogical const&) = 0;
	virtual void visit(ExprInvoke const&) = 0;
	virtual void visit(ExprGet const&) = 0;
	virtual void visit(ExprSet const&) = 0;
};

struct Expr::Printer : Visitor {
	std::string& out;

	Printer(std::string& out) : out(out) {}

	void visit(ExprLiteral const& expr) override final;
	void visit(ExprGroup const& expr) override final;
	void visit(ExprUnary const& expr) override final;
	void visit(ExprBinary const& expr) override final;
	void visit(ExprVar const& expr) override final;
	void visit(ExprAssign const& expr) override final;
	void visit(ExprLogical const& expr) override final;
	void visit(ExprInvoke const& expr) override final;
	void visit(ExprGet const& expr) override final;
	void visit(ExprSet const& expr) override final;
};

std::string to_string(Expr const& expr);
} // namespace toylang
