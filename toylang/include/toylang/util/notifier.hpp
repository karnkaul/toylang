#pragma once
#include <toylang/diagnostic.hpp>
#include <memory>

namespace toylang::util {
class Notifier {
  public:
	Notifier() noexcept;
	Notifier(Notifier&&) noexcept;
	Notifier& operator=(Notifier&&) noexcept;
	virtual ~Notifier() noexcept;

	Notifier(std::unique_ptr<Notifier>&& next) noexcept;

	void notify(Diagnostic const& diag);
	void operator()(Diagnostic const& diag) { notify(diag); }

  protected:
	virtual void on_notify(Diagnostic const& diag) = 0;

	std::unique_ptr<Notifier> m_next{};
};
} // namespace toylang::util
