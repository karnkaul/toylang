#pragma once
#include <toylang/environment.hpp>
#include <toylang/media.hpp>
#include <toylang/source.hpp>
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

	bool execute(Source program);
	bool evaluate(std::string_view expression);
	bool execute_or_evaluate(Source source);

	Environment& environment() { return m_environment; }

	void runtime_error(Token const& at, std::string_view message) const;
	void clear_state();

	Media media{};
	Debug debug{};

  private:
	struct Eval;
	struct Exec;
	struct Storage {
		std::vector<util::CharBuf> texts{};
		std::vector<UStmt> executed{};
		std::vector<std::string> imported{};

		void clear() {
			texts.clear();
			executed.clear();
			imported.clear();
		}
	};

	bool execute_import(Token const& path);
	bool is_errored() const { return m_reporter->error(); }
	bool define(Token const& name, Value value);
	bool assign(Token const& name, Value value);

	template <typename... T>
	void add_intrinsic();
	void add_intrinsics();
	Source store(Source source);
	Stmt& store(UStmt&& stmt);

	std::unique_ptr<util::Reporter> m_reporter{};
	Storage m_storage{};
	Environment m_environment{};
};
} // namespace toylang
