#pragma once
#include <toylang/util/notifier.hpp>

namespace toylang::util {
class Reporter : public Notifier {
  public:
	struct Context {
		std::string_view filename{};
		std::string_view full_text{};
	};

	Reporter(std::unique_ptr<Notifier> next) : Notifier{std::move(next)} {}

	void reset(Context context) { m_data = {context}; }

	void set_error() { m_data.error = true; }
	bool error() const { return m_data.error; }

	char quote = '\'';
	char mark = '^';

  private:
	void on_notify(Diagnostic const& diag) override;

	struct {
		Context context{};
		bool error{};
	} m_data{};
};
} // namespace toylang::util
