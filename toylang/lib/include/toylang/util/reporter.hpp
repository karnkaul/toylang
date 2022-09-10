#pragma once
#include <toylang/util/notifier.hpp>

namespace toylang::util {
///
/// \brief Concrete Notifier: prints formatted messages to stderr
///
class Reporter : public Notifier {
  public:
	Reporter(std::unique_ptr<Notifier> next) : Notifier{std::move(next)} {}

	void reset(std::string_view filename) { m_data = {filename}; }

	void set_error() { m_data.error = true; }
	bool error() const { return m_data.error; }

	char quote = '\'';
	char mark = '^';

  private:
	void on_notify(Diagnostic const& diag) override;

	struct {
		std::string_view filename{};
		bool error{};
	} m_data{};
};
} // namespace toylang::util
