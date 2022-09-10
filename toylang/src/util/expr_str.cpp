#include <toylang/util.hpp>
#include <toylang/util/expr_str.hpp>

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

Value ExprStr::visit(ExprLiteral const& expr) {
	switch (expr.value.type()) {
	case Literal::Type::eNull: util::append(out, "null"); break;
	case Literal::Type::eBool: util::append(out, expr.value.as_bool() ? "true" : "false"); break;
	case Literal::Type::eString: util::append(out, expr.value.as_string()); break;
	case Literal::Type::eDouble: util::append(out, std::to_string(expr.value.as_double())); break;
	}
	return {};
}

Value ExprStr::visit(ExprGroup const& expr) {
	auto p = Parenthesize{out};
	if (expr.expr) { expr.accept(*this); }
	return {};
}

Value ExprStr::visit(ExprUnary const& expr) {
	auto p = Parenthesize{out};
	append(out, expr.op);
	assert(expr.rhs);
	expr.rhs->accept(*this);
	return {};
}

Value ExprStr::visit(ExprBinary const& expr) {
	auto p = Parenthesize{out};
	assert(expr.lhs);
	expr.lhs->accept(*this);
	append(out, expr.op);
	assert(expr.rhs);
	expr.rhs->accept(*this);
	return {};
}

Value ExprStr::visit(ExprVar const& expr) {
	append(out, expr.name.lexeme);
	return {};
}

Value ExprStr::visit(ExprAssign const& expr) {
	auto p = Parenthesize{out};
	append(out, expr.name.lexeme);
	append(out, token_string(TokenType::eEq));
	assert(expr.value);
	expr.value->accept(*this);
	return {};
}

Value ExprStr::visit(ExprLogical const& expr) {
	auto p = Parenthesize{out};
	assert(expr.lhs);
	expr.lhs->accept(*this);
	append(out, expr.op);
	assert(expr.rhs);
	expr.rhs->accept(*this);
	return {};
}

Value ExprStr::visit(ExprInvoke const& expr) {
	assert(expr.callee);
	expr.callee->accept(*this);
	auto p = Parenthesize{out};
	auto first = true;
	for (auto const& arg : expr.args.args) {
		if (!arg) { break; }
		if (!first) { util::append(out, ", "); }
		arg->accept(*this);
	}
	return {};
}

Value ExprStr::visit(ExprGet const& expr) {
	assert(expr.obj);
	expr.obj->accept(*this);
	util::append(out, ".", expr.name.lexeme);
	return {};
}

Value ExprStr::visit(ExprSet const& expr) {
	assert(expr.obj && expr.value);
	expr.obj->accept(*this);
	util::append(out, ".", expr.name.lexeme, " = ");
	expr.value->accept(*this);
	return {};
}
} // namespace toylang
