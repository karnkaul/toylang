#pragma once
#include <toylang/value.hpp>
#include <unordered_map>

namespace toylang {
///
/// \brief The approach to Environment in toylang is significantly different from that in Crafting Interpreters:
/// Here, each lexical scope has an associated Page, where nested scopes are stored as successive entries in a Chapter.
/// A function call imitates a new frame, by pushing a new Chapter on the existing stack - previous chapters **are not** traversed.
/// This means a function calls will have its own dedicated environment: globals and parameters only.
///
class Environment {
  public:
	class Scope;
	class Frame;

	Environment();

	bool assign(std::string_view const& key, Value value);
	bool define(std::string_view key, Value value);
	Value* find(std::string_view const& key);

	std::size_t depth() const { return m_book.size(); }

  private:
	using Page = std::unordered_map<std::string_view, Value>;
	using Chapter = std::vector<Page>;

	static Chapter make_chapter() { return Chapter{1, Page{}}; }

	Page& top();
	Page& global();

	void begin_scope();
	void end_scope();
	void push_frame();
	void pop_frame();

	std::vector<Chapter> m_book{};
};

class Environment::Scope {
  public:
	Scope(Environment& environment) : m_environment(environment) { m_environment.begin_scope(); }
	~Scope() noexcept { m_environment.end_scope(); }

	Scope& operator=(Scope&&) = delete;

  private:
	Environment& m_environment;
};

class Environment::Frame {
  public:
	Frame(Environment& environment) : m_environment(environment) { m_environment.push_frame(); }
	~Frame() noexcept { m_environment.pop_frame(); }

	Frame& operator=(Frame&&) = delete;

  private:
	Environment& m_environment;
};
} // namespace toylang
