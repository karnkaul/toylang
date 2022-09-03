#include <toylang/expr.hpp>

namespace toylang {
namespace {
struct Parenthesize {
	std::string& out;

	Parenthesize(std::string& out) : out(out) { out += "("; }
	~Parenthesize() { out += ")"; }
};

void append(std::string& out, Token const& op) {
	out += ' ';
	out += op.lexeme;
	out += ' ';
}
} // namespace

void LiteralExpr::accept(Visitor& out) const { out.visit(*this); }
void GroupExpr::accept(Visitor& out) const { out.visit(*this); }
void UnaryExpr::accept(Visitor& out) const { out.visit(*this); }
void BinaryExpr::accept(Visitor& out) const { out.visit(*this); }

void Expr::Printer::visit(LiteralExpr const& expr) {
	switch (expr.value.type()) {
	case Trivial::Type::eNull: out += "null"; break;
	case Trivial::Type::eBool: out += (expr.value.as_bool() ? "true" : "false"); break;
	case Trivial::Type::eString: out += expr.value.as_string(); break;
	case Trivial::Type::eDouble: out += std::to_string(expr.value.as_double()); break;
	}
}

void Expr::Printer::visit(GroupExpr const& expr) {
	auto p = Parenthesize{out};
	if (expr.expr) { expr.accept(*this); }
}

void Expr::Printer::visit(UnaryExpr const& expr) {
	auto p = Parenthesize{out};
	append(out, expr.op);
	assert(expr.right);
	expr.right->accept(*this);
}

void Expr::Printer::visit(BinaryExpr const& expr) {
	auto p = Parenthesize{out};
	assert(expr.left);
	expr.left->accept(*this);
	append(out, expr.op);
	assert(expr.right);
	expr.right->accept(*this);
}

std::string to_string(Expr const& expr) {
	auto ret = std::string{};
	auto printer = Expr::Printer{ret};
	expr.accept(printer);
	return ret;
}
} // namespace toylang
