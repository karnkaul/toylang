#pragma once
#include <toylang/environment.hpp>
#include <toylang/stmt.hpp>
#include <toylang/util/buffer.hpp>
#include <toylang/util/reporter.hpp>

namespace toylang {
class Interpreter {
  public:
	enum : std::uint32_t { ePrintStmtExprs = 1 << 0 };
	using Debug = std::uint32_t;

	Interpreter(std::unique_ptr<util::Notifier> custom = {});

	Interpreter& operator=(Interpreter&&) = delete;

	bool execute(std::string_view program);
	bool evaluate(std::string_view expression);
	bool execute_or_evaluate(std::string_view text);

	Environment& environment() { return m_environment; }

	bool is_reserved(std::string_view name) const;
	void runtime_error(Token const& at, std::string_view message) const;
	void clear_state();

	Debug debug{};

  private:
	struct Eval;
	struct Exec;
	struct Storage {
		std::vector<util::CharBuf> texts{};
		std::vector<UStmt> executed{};

		void clear() {
			texts.clear();
			executed.clear();
		}
	};

	bool is_errored() const { return m_reporter->error(); }
	bool require_unreserved(Token const& name) const;
	bool define(Token const& name, Value value);
	bool assign(Token const& name, Value value);

	void add_intrinsics();
	std::string_view store(util::Reporter::Context context);
	Stmt& store(UStmt&& stmt);

	std::unique_ptr<util::Reporter> m_reporter{};
	Storage m_storage{};
	Environment m_environment{};
};
} // namespace toylang
