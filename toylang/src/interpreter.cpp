#include <internal/intrinsics.hpp>
#include <toylang/interpreter.hpp>
#include <toylang/parser.hpp>
#include <toylang/stmt.hpp>
#include <toylang/util.hpp>
#include <compare>
#include <cstdio>
#include <span>
#include <utility>

namespace toylang {
namespace {
struct EvalError {};

Diagnostic make_diagnostic(Token const& token, std::string_view message, TokenType expected, Diagnostic::Type type) {
	return Diagnostic{
		.token = token,
		.expected = expected,
		.message = message,
		.type = type,
	};
}

Diagnostic make_internal_error(Token const& token, std::string_view message, TokenType expected = TokenType::eEof) {
	return make_diagnostic(token, message, expected, Diagnostic::Type::eInternalError);
}

Diagnostic make_runtime_error(Token const& token, std::string_view message, TokenType expected = TokenType::eEof) {
	return make_diagnostic(token, message, expected, Diagnostic::Type::eRuntimeError);
}

void expect_assignable(util::Reporter* reporter, Token const& name, Value const& value) noexcept(false) {
	if (value.contains<StructDef>()) {
		if (reporter) { (*reporter)(make_runtime_error(name, "Cannot initialize variable as a struct")); }
		throw EvalError{};
	}
}
} // namespace

struct Interpreter::Eval : Expr::Visitor {
	Value value{};
	Interpreter& interpreter;

	Eval(Interpreter& interpreter);

	static Eval evaluate(Interpreter& interpreter, Expr const* expr);

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

	void evaluate(Expr const& expr);

	bool try_arithmetic(Eval const& lhs, Eval const& rhs, ExprBinary const& expr);
	bool try_equality(Eval const& lhs, Eval const& rhs, ExprBinary const& expr);
	bool try_comparison(Eval const& lhs, Eval const& rhs, ExprBinary const& expr);

	void expect_number(Token const& op, Eval const& lhs, Eval const& rhs) const noexcept(false);
};

struct Interpreter::Exec : Stmt::Visitor {
	Interpreter& interpreter;

	Exec(Interpreter& interpreter);

	void visit(StmtExpr const& stmt) override final;
	void visit(StmtVar const& stmt) override final;
	void visit(StmtBlock const& stmt) override final;
	void visit(StmtIf const& stmt) override final;
	void visit(StmtWhile const& stmt) override final;
	void visit(StmtBreak const& stmt) override final;
	void visit(StmtFn const& stmt) override final;
	void visit(StmtReturn const& stmt) override final;
	void visit(StmtStruct const& stmt) override final;

	Value evaluate(Expr const* expr);
	void execute(Stmt const& stmt);
	bool check_statement(Stmt const* stmt) const;
	void execute_block(std::span<UStmt const> stmt);
};

Interpreter::Eval::Eval(Interpreter& interpreter) : interpreter(interpreter) {}

Interpreter::Eval Interpreter::Eval::evaluate(Interpreter& interprter, Expr const* expr) {
	auto ret = Eval{interprter};
	if (expr) { expr->accept(ret); }
	return ret;
}

void Interpreter::Eval::evaluate(Expr const& expr) { expr.accept(*this); }

void Interpreter::Eval::visit(ExprLiteral const& expr) { value = Value::make(expr.value); }

void Interpreter::Eval::visit(ExprGroup const& expr) {
	if (expr.expr) { evaluate(*expr.expr); }
}

void Interpreter::Eval::visit(ExprUnary const& expr) {
	if (!expr.rhs) {
		if (interpreter.m_reporter) { (*interpreter.m_reporter)(make_internal_error(expr.op, "Right operand does not exist")); }
		return;
	}
	evaluate(*expr.rhs);
	switch (expr.op.type) {
	case TokenType::eMinus: {
		if (!value.contains<double>()) {
			if (interpreter.m_reporter) { (*interpreter.m_reporter)(make_internal_error(expr.op, "Invalid operand to unary expression", TokenType::eNumber)); }
			break;
		}
		auto& d = value.get<double>();
		d = -d;
		break;
	}
	case TokenType::eBang: {
		value.payload = Bool{!value.is_truthy()};
		break;
	}
	default: {
		if (interpreter.m_reporter) { (*interpreter.m_reporter)(make_internal_error(expr.op, "Unexpected unary operator")); }
		break;
	}
	}
}

void Interpreter::Eval::visit(ExprBinary const& expr) {
	if (!expr.lhs || !expr.rhs) {
		if (interpreter.m_reporter) { (*interpreter.m_reporter)(make_internal_error(expr.op, "Left/right operands do not exist")); }
		return;
	}
	auto lhs = evaluate(interpreter, expr.lhs.get());
	auto rhs = evaluate(interpreter, expr.rhs.get());
	if (try_arithmetic(lhs, rhs, expr)) { return; }
	if (try_equality(lhs, rhs, expr)) { return; }
	if (try_comparison(lhs, rhs, expr)) { return; }
	// TODO
}

void Interpreter::Eval::visit(ExprVar const& expr) {
	auto* bound = interpreter.m_environment.find(std::string{expr.name.lexeme});
	if (!bound) {
		if (interpreter.m_reporter) { (*interpreter.m_reporter)(make_runtime_error(expr.name, "Undefined variable")); }
		throw EvalError{};
	}
	value = *bound;
}

void Interpreter::Eval::visit(ExprAssign const& expr) {
	auto* bound = interpreter.m_environment.find(std::string{expr.name.lexeme});
	if (!bound) {
		if (interpreter.m_reporter) { (*interpreter.m_reporter)(make_runtime_error(expr.name, "Undefined variable")); }
		return;
	}
	*bound = std::move(evaluate(interpreter, expr.value.get()).value);
	expect_assignable(interpreter.m_reporter.get(), expr.name, *bound);
	value = *bound;
}

void Interpreter::Eval::visit(ExprLogical const& expr) {
	if (!expr.lhs || !expr.rhs) {
		if (interpreter.m_reporter) { (*interpreter.m_reporter)(make_internal_error(expr.op, "Operand(s) don't exist")); }
		return;
	}
	auto lhs = evaluate(interpreter, expr.lhs.get());
	if (expr.op.type == TokenType::eOr) {
		if (lhs.value.is_truthy()) {
			value = std::move(lhs.value);
			return;
		}
	} else if (expr.op.type == TokenType::eAnd) {
		if (!lhs.value.is_truthy()) {
			value = std::move(lhs.value);
			return;
		}
	}

	value = std::move(evaluate(interpreter, expr.rhs.get()).value);
}

void Interpreter::Eval::visit(ExprInvoke const& expr) {
	if (!expr.callee) {
		if (interpreter.m_reporter) { (*interpreter.m_reporter)(make_internal_error(expr.paren_r, "Callee doesn't exist")); }
		return;
	}
	auto callee = std::move(evaluate(interpreter, expr.callee.get()).value);
	if (!callee.contains<Invocable>() && !callee.contains<StructDef>()) {
		if (interpreter.m_reporter) { (*interpreter.m_reporter)(make_runtime_error(expr.paren_r, "Invalid callee")); }
		return;
	}
	auto args = std::vector<Value>{};
	args.reserve(expr.args.arity);
	for (auto const& arg : expr.args.args) {
		if (!arg) { break; }
		args.push_back(std::move(evaluate(interpreter, arg.get()).value));
	}
	if (callee.contains<Invocable>()) {
		auto const& cb = callee.get<Invocable>().callback;
		if (!cb) {
			if (interpreter.m_reporter) { (*interpreter.m_reporter)(make_internal_error(expr.paren_r, "Invocable doesn't exist")); }
			return;
		}

		value = cb(interpreter, {expr.paren_r, args});
	} else {
		auto const& sd = callee.get<StructDef>();
		value.payload = sd.instance();
	}
}

void Interpreter::Eval::visit(ExprGet const& expr) {
	if (!expr.obj) {
		if (interpreter.m_reporter) { (*interpreter.m_reporter)(make_internal_error(expr.name, "Caller doesn't exist")); }
		return;
	}
	evaluate(*expr.obj);
	if (!value.contains<StructInst>()) {
		interpreter.runtime_error(expr.name, "Only instances have properties");
		throw EvalError{};
	}
	auto const* field = value.get<StructInst>().find(expr.name.lexeme);
	if (!field) {
		interpreter.runtime_error(expr.name, "Undefined property");
		throw EvalError{};
	}
	value = *field;
}

void Interpreter::Eval::visit(ExprSet const& expr) {
	if (!expr.obj.get()) {
		if (interpreter.m_reporter) { (*interpreter.m_reporter)(make_internal_error(expr.name, "Object doesn't exist")); }
		throw EvalError{};
	}
	if (!expr.value.get()) {
		if (interpreter.m_reporter) { (*interpreter.m_reporter)(make_internal_error(expr.name, "Value doesn't exist")); }
		throw EvalError{};
	}
	evaluate(*expr.obj);
	if (!value.contains<StructInst>()) {
		interpreter.runtime_error(expr.name, "Only instances have fields");
		throw EvalError{};
	}
	auto inst = value.get<StructInst>();
	evaluate(*expr.value);
	if (!inst.set(expr.name.lexeme, std::move(value))) {
		if (interpreter.m_reporter) { (*interpreter.m_reporter)(make_internal_error(expr.name, "Failed to set field")); }
		throw EvalError{};
	}
}

bool Interpreter::Eval::try_arithmetic(Eval const& lhs, Eval const& rhs, ExprBinary const& expr) {
	switch (expr.op.type) {
	case TokenType::eMinus: {
		expect_number(expr.op, lhs, rhs);
		value = Value::make(lhs.value.get<double>() - rhs.value.get<double>());
		return true;
	}
	case TokenType::eStar: {
		expect_number(expr.op, lhs, rhs);
		value = Value::make(lhs.value.get<double>() * rhs.value.get<double>());
		return true;
	}
	case TokenType::eSlash: {
		expect_number(expr.op, lhs, rhs);
		value = Value::make(lhs.value.get<double>() / rhs.value.get<double>());
		return true;
	}
	case TokenType::ePlus: {
		if (lhs.value.contains<double>() && rhs.value.contains<double>()) {
			value.payload = lhs.value.get<double>() + rhs.value.get<double>();
		} else if (lhs.value.contains<std::string>() && rhs.value.contains<std::string>()) {
			value.payload = std::move(lhs.value.get<std::string>()) + std::move(rhs.value.get<std::string>());
		} else {
			if (interpreter.m_reporter) { (*interpreter.m_reporter)(make_runtime_error(expr.op, "Invalid operands to binary expression")); }
			throw EvalError{};
		}
		return true;
	}
	default: return false;
	}
}

bool Interpreter::Eval::try_equality(Eval const& lhs, Eval const& rhs, ExprBinary const& expr) {
	switch (expr.op.type) {
	case TokenType::eEqEq: {
		value.payload = Bool{lhs.value == rhs.value};
		return true;
	}
	case TokenType::eBangEq: {
		value.payload = Bool{lhs.value != rhs.value};
		return true;
	}
	default: return false;
	}
}

bool Interpreter::Eval::try_comparison(Eval const& lhs, Eval const& rhs, ExprBinary const& expr) {
	auto compare = [this, op = expr.op](Value const& a, Value const& b) -> std::partial_ordering {
		bool const are_str = a.contains<std::string>() && b.contains<std::string>();
		bool const are_double = a.contains<double>() && b.contains<double>();
		if (!are_str) {
			if (!are_double) {
				if (interpreter.m_reporter) { (*interpreter.m_reporter)(make_runtime_error(op, "Invalid operands to binary expression")); }
				throw EvalError{};
			}
			return a.get<double>() <=> b.get<double>();
		}
		return a.get<std::string>() <=> b.get<std::string>();
	};
	switch (expr.op.type) {
	case TokenType::eLt: {
		value.payload = Bool{compare(lhs.value, rhs.value) < 0};
		return true;
	}
	case TokenType::eLe: {
		value.payload = Bool{compare(lhs.value, rhs.value) <= 0};
		return true;
	}
	case TokenType::eGt: {
		value.payload = Bool{compare(lhs.value, rhs.value) > 0};
		return true;
	}
	case TokenType::eGe: {
		value.payload = Bool{compare(lhs.value, rhs.value) >= 0};
		return true;
	}
	default: return false;
	}
}

void Interpreter::Eval::expect_number(Token const& op, Eval const& lhs, Eval const& rhs) const {
	if (!lhs.value.contains<double>() || !rhs.value.contains<double>()) {
		if (interpreter.m_reporter) { (*interpreter.m_reporter)(make_runtime_error(op, "Invalid operands to binary expression", TokenType::eNumber)); }
		throw EvalError{};
	}
}

Interpreter::Exec::Exec(Interpreter& interpreter) : interpreter(interpreter) {}

void Interpreter::Exec::visit(StmtExpr const& stmt) {
	auto const value = evaluate(stmt.expr.get());
	if ((interpreter.debug & ePrintStmtExprs) == ePrintStmtExprs) { std::printf("[Debug] %s\n", to_string(value).c_str()); }
}

void Interpreter::Exec::visit(StmtVar const& stmt) {
	if (!interpreter.require_unreserved(stmt.name)) { return; }
	auto value = evaluate(stmt.initializer.get());
	expect_assignable(interpreter.m_reporter.get(), stmt.name, value);
	interpreter.define(stmt.name, std::move(value));
}

void Interpreter::Exec::visit(StmtBlock const& stmt) {
	auto scope = Environment::Scope{interpreter.m_environment};
	execute_block(stmt.statements);
}

void Interpreter::Exec::visit(StmtIf const& stmt) {
	if (evaluate(stmt.condition.get()).is_truthy()) {
		if (stmt.on) { execute(*stmt.on); }
	} else {
		if (stmt.off) { execute(*stmt.off); }
	}
}

void Interpreter::Exec::visit(StmtWhile const& stmt) {
	try {
		while (evaluate(stmt.condition.get()).is_truthy()) { stmt.body->accept(*this); }
	} catch (StmtBreak::Break const& brk) {}
}

void Interpreter::Exec::visit(StmtBreak const& stmt) { throw StmtBreak::Break{stmt.brk}; }

void Interpreter::Exec::visit(StmtFn const& stmt) {
	if (!interpreter.require_unreserved(stmt.name)) { return; }
	struct Invoker {
		StmtFn const* decl{};
		util::Notifier* notifier{};

		Value operator()(Interpreter& in, CallContext ctx) {
			auto& [callee, values] = ctx;
			// auto scope = Environment2::Scope{in.m_environment};
			auto stack_frame = Environment::Frame{in.m_environment};
			if (decl->params.arity != values.size()) {
				if (notifier) {
					auto err = std::string{"Mismatched argument count: expected "};
					util::append(err, std::to_string(decl->params.arity), " passed: ", std::to_string(values.size()));
					(*notifier)(make_runtime_error(callee, err));
				}
				return {};
			}
			for (std::size_t i = 0; i < values.size(); ++i) { in.define(decl->params.args[i], std::move(values[i])); }
			try {
				Exec{in}.execute_block(decl->body);
			} catch (StmtReturn::Return const&) {
				if (auto* ret = in.m_environment.find("<ret>")) { return std::move(*ret); }
			}
			return {};
		}
	};
	auto value = Value{};
	value.payload = Invocable{stmt.name, Invoker{&stmt, interpreter.m_reporter.get()}};
	interpreter.define(stmt.name, std::move(value));
}

void Interpreter::Exec::visit(StmtReturn const& stmt) {
	if (stmt.ret) { interpreter.m_environment.define("<ret>", evaluate(stmt.ret.get())); }
	throw StmtReturn::Return{stmt.token};
}

void Interpreter::Exec::visit(StmtStruct const& stmt) {
	auto def = StructDef{.name = stmt.name.lexeme};
	for (auto const& var : stmt.vars) { def.fields.push_back(var->name.lexeme); }
	interpreter.define(stmt.name, {.payload = std::move(def)});
	// TODO
}

void Interpreter::Exec::execute(Stmt const& stmt) {
	if (!interpreter.is_errored()) {
		try {
			stmt.accept(*this);
		} catch (EvalError const&) { interpreter.m_reporter->set_error(); }
	}
}

bool Interpreter::Exec::check_statement(Stmt const* stmt) const {
	if (!stmt) {
		if (interpreter.m_reporter) {
			(*interpreter.m_reporter)(make_internal_error({}, "Statement does not exist"));
			return false;
		}
	}
	return true;
}

Value Interpreter::Exec::evaluate(Expr const* expr) { return std::move(Eval::evaluate(interpreter, expr).value); }

void Interpreter::Exec::execute_block(std::span<UStmt const> stmts) {
	for (auto const& stmt : stmts) {
		if (!check_statement(stmt.get())) { continue; }
		execute(*stmt);
	}
}

Interpreter::Interpreter(std::unique_ptr<util::Notifier> custom) : m_reporter{std::make_unique<util::Reporter>(std::move(custom))} { add_intrinsics(); }

bool Interpreter::execute_or_evaluate(std::string_view text) {
	if (Parser::is_expression(text)) { return evaluate(text); }
	return execute(text);
}

bool Interpreter::execute(std::string_view program) {
	if (program.empty()) { return true; }
	program = store({.full_text = program});
	auto parser = Parser{program, m_reporter.get()};
	auto exec = Exec{*this};
	while (auto stmt = parser.parse_stmt()) {
		try {
			exec.execute(*stmt);
		} catch (StmtBreak::Break const& brk) {
			m_reporter->notify(make_runtime_error(brk.token, "Unexpected break outside of any loops"));
		} catch (StmtReturn::Return const& ret) { m_reporter->notify(make_runtime_error(ret.token, "Unexpected return outside of any functions")); }
		store(std::move(stmt));
	}
	return !is_errored();
}

bool Interpreter::evaluate(std::string_view expression) {
	if (expression.empty()) { return false; }
	expression = store({.full_text = expression});
	auto parser = Parser{expression, m_reporter.get()};
	auto eval = Eval{*this};
	while (auto expr = parser.parse_expr()) {
		expr->accept(eval);
		std::printf("%s\n", util::unescape(to_string(eval.value)).c_str());
	}
	return !is_errored();
}

bool Interpreter::is_reserved(std::string_view name) const { return intrinsics::taken(name); }

void Interpreter::runtime_error(Token const& at, std::string_view message) const { m_reporter->notify(make_runtime_error(at, message)); }

void Interpreter::clear_state() {
	m_environment = Environment{};
	m_storage.clear();
	m_reporter.reset({});
}

bool Interpreter::require_unreserved(Token const& name) const {
	if (is_reserved(name.lexeme)) {
		m_reporter->notify(make_runtime_error(name, "Cannot overwwrite intrinsic"));
		return false;
	}
	return true;
}

bool Interpreter::define(Token const& name, Value value) {
	if (!require_unreserved(name)) { return false; }
	m_environment.define(name.lexeme, std::move(value));
	return true;
}

bool Interpreter::assign(Token const& name, Value value) {
	if (!require_unreserved(name)) { return false; }
	m_environment.assign(name.lexeme, std::move(value));
	return true;
}

void Interpreter::add_intrinsics() {
	m_environment.define(intrinsics::Print::name_v, {.payload = Invocable{.callback = intrinsics::Print{}}});
	m_environment.define(intrinsics::PrintF::name_v, {.payload = Invocable{.callback = intrinsics::PrintF{}}});
	m_environment.define(intrinsics::Clone::name_v, {.payload = Invocable{.callback = intrinsics::Clone{}}});
}

std::string_view Interpreter::store(util::Reporter::Context context) {
	assert(!context.full_text.empty());
	if (!context.filename.empty()) {
		m_storage.texts.push_back(context.filename);
		context.filename = m_storage.texts.back();
	}
	m_storage.texts.push_back(context.full_text);
	context.full_text = m_storage.texts.back();
	m_reporter->reset(context);
	return context.full_text;
}

Stmt& Interpreter::store(UStmt&& stmt) {
	assert(stmt);
	m_storage.executed.push_back(std::move(stmt));
	return *m_storage.executed.back();
}
} // namespace toylang
