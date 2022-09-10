#include <toylang/environment.hpp>
#include <utility>

namespace toylang {
Environment::Environment() { m_book.push_back(make_chapter()); }

bool Environment::assign(std::string_view const& key, Value value) {
	if (auto* target = find(key)) {
		*target = std::move(value);
		return true;
	}
	return false;
}

bool Environment::define(std::string_view key, Value value) {
	top().insert_or_assign(std::move(key), std::move(value));
	return true;
}

Value* Environment::find(std::string_view const& key) {
	auto search = [key](Page& store) -> Value* {
		if (auto it = store.find(key); it != store.end()) { return &it->second; }
		return {};
	};
	assert(!m_book.empty());
	auto& active = m_book.back();
	for (auto it = active.rbegin(); it != active.rend(); ++it) {
		if (auto ret = search(*it)) { return ret; }
	}
	return depth() > 0 ? search(global()) : nullptr;
}

void Environment::begin_scope() {
	assert(!m_book.empty());
	m_book.back().emplace_back();
}

void Environment::end_scope() {
	assert(!m_book.empty() && !m_book.back().empty());
	m_book.back().pop_back();
}

void Environment::push_frame() { m_book.push_back(make_chapter()); }

void Environment::pop_frame() {
	assert(!m_book.empty());
	m_book.pop_back();
}

Environment::Page& Environment::global() {
	assert(!m_book.empty() && !m_book.front().empty());
	return m_book.front().front();
}

Environment::Page& Environment::top() {
	assert(!m_book.empty() && !m_book.back().empty());
	return m_book.back().back();
}
} // namespace toylang
