#pragma once
#include <toylang/expr.hpp>

namespace toylang {
struct ExprStr : Expr::Visitor {
	std::string& out;

	ExprStr(std::string& out) : out(out) {}

	Value visit(ExprLiteral const& expr) override final;
	Value visit(ExprGroup const& expr) override final;
	Value visit(ExprUnary const& expr) override final;
	Value visit(ExprBinary const& expr) override final;
	Value visit(ExprVar const& expr) override final;
	Value visit(ExprAssign const& expr) override final;
	Value visit(ExprLogical const& expr) override final;
	Value visit(ExprInvoke const& expr) override final;
	Value visit(ExprGet const& expr) override final;
	Value visit(ExprSet const& expr) override final;
};
} // namespace toylang
