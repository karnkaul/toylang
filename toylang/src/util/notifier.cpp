#include <toylang/util/notifier.hpp>

namespace toylang::util {
Notifier::Notifier() noexcept = default;
Notifier::Notifier(Notifier&&) noexcept = default;
Notifier& Notifier::operator=(Notifier&&) noexcept = default;
Notifier::~Notifier() noexcept = default;

Notifier::Notifier(std::unique_ptr<Notifier>&& next) noexcept : m_next{std::move(next)} {}

void Notifier::notify(Diagnostic const& diag) {
	on_notify(diag);
	if (m_next) { m_next->notify(diag); }
}
} // namespace toylang::util
