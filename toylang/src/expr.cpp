#include <toylang/expr.hpp>
#include <toylang/util.hpp>
#include <toylang/util/expr_str.hpp>

namespace toylang {
Value ExprLiteral::accept(Visitor& out) const { return out.visit(*this); }
Value ExprGroup::accept(Visitor& out) const { return out.visit(*this); }
Value ExprUnary::accept(Visitor& out) const { return out.visit(*this); }
Value ExprBinary::accept(Visitor& out) const { return out.visit(*this); }
Value ExprVar::accept(Visitor& out) const { return out.visit(*this); }
Value ExprAssign::accept(Visitor& out) const { return out.visit(*this); }
Value ExprLogical::accept(Visitor& out) const { return out.visit(*this); }
Value ExprInvoke::accept(Visitor& out) const { return out.visit(*this); }
Value ExprGet::accept(Visitor& out) const { return out.visit(*this); }
Value ExprSet::accept(Visitor& out) const { return out.visit(*this); }

std::string to_string(Expr const& expr) {
	auto ret = std::string{};
	auto str = ExprStr{ret};
	expr.accept(str);
	return ret;
}
} // namespace toylang
