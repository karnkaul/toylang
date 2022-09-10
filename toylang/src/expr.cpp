#include <toylang/expr.hpp>
#include <toylang/util.hpp>

namespace toylang {
namespace {
struct Parenthesize {
	std::string& out;

	Parenthesize(std::string& out) : out(out) { util::append(out, "("); }
	~Parenthesize() { util::append(out, ")"); }
};

void append(std::string& out, std::string_view const text) { util::append(out, " ", text, " "); }

void append(std::string& out, Token const& op) { append(out, op.lexeme); }
} // namespace

void ExprLiteral::accept(Visitor& out) const { out.visit(*this); }
void ExprGroup::accept(Visitor& out) const { out.visit(*this); }
void ExprUnary::accept(Visitor& out) const { out.visit(*this); }
void ExprBinary::accept(Visitor& out) const { out.visit(*this); }
void ExprVar::accept(Visitor& out) const { out.visit(*this); }
void ExprAssign::accept(Visitor& out) const { out.visit(*this); }
void ExprLogical::accept(Visitor& out) const { out.visit(*this); }
void ExprInvoke::accept(Visitor& out) const { out.visit(*this); }
void ExprGet::accept(Visitor& out) const { out.visit(*this); }
void ExprSet::accept(Visitor& out) const { out.visit(*this); }

void Expr::Printer::visit(ExprLiteral const& expr) {
	switch (expr.value.type()) {
	case Literal::Type::eNull: util::append(out, "null"); break;
	case Literal::Type::eBool: util::append(out, expr.value.as_bool() ? "true" : "false"); break;
	case Literal::Type::eString: util::append(out, expr.value.as_string()); break;
	case Literal::Type::eDouble: util::append(out, std::to_string(expr.value.as_double())); break;
	}
}

void Expr::Printer::visit(ExprGroup const& expr) {
	auto p = Parenthesize{out};
	if (expr.expr) { expr.accept(*this); }
}

void Expr::Printer::visit(ExprUnary const& expr) {
	auto p = Parenthesize{out};
	append(out, expr.op);
	assert(expr.rhs);
	expr.rhs->accept(*this);
}

void Expr::Printer::visit(ExprBinary const& expr) {
	auto p = Parenthesize{out};
	assert(expr.lhs);
	expr.lhs->accept(*this);
	append(out, expr.op);
	assert(expr.rhs);
	expr.rhs->accept(*this);
}

void Expr::Printer::visit(ExprVar const& expr) { append(out, expr.name.lexeme); }

void Expr::Printer::visit(ExprAssign const& expr) {
	auto p = Parenthesize{out};
	append(out, expr.name.lexeme);
	append(out, token_string(TokenType::eEq));
	assert(expr.value);
	expr.value->accept(*this);
}

void Expr::Printer::visit(ExprLogical const& expr) {
	auto p = Parenthesize{out};
	assert(expr.lhs);
	expr.lhs->accept(*this);
	append(out, expr.op);
	assert(expr.rhs);
	expr.rhs->accept(*this);
}

void Expr::Printer::visit(ExprInvoke const& expr) {
	assert(expr.callee);
	expr.callee->accept(*this);
	auto p = Parenthesize{out};
	auto first = true;
	for (auto const& arg : expr.args.args) {
		if (!arg) { break; }
		if (!first) { util::append(out, ", "); }
		arg->accept(*this);
	}
}

void Expr::Printer::visit(ExprGet const& expr) {
	assert(expr.obj);
	expr.obj->accept(*this);
	util::append(out, ".", expr.name.lexeme);
}

void Expr::Printer::visit(ExprSet const& expr) {
	assert(expr.obj && expr.value);
	expr.obj->accept(*this);
	util::append(out, ".", expr.name.lexeme, " = ");
	expr.value->accept(*this);
}

std::string to_string(Expr const& expr) {
	auto ret = std::string{};
	auto printer = Expr::Printer{ret};
	expr.accept(printer);
	return ret;
}
} // namespace toylang
