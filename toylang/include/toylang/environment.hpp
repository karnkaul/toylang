#pragma once
#include <toylang/value.hpp>
#include <unordered_map>

namespace toylang {
namespace old {
class Environment {
  public:
	class Enclosed;

	virtual ~Environment() = default;

	bool define(std::string_view key, Value value);
	bool destroy(std::string_view const& key);

	virtual bool assign(std::string_view const& key, Value value);
	virtual Value const* find(std::string_view const& key) const;
	virtual Value* find(std::string_view const& key);

	std::size_t count() const;
	void clear();

  private:
	std::unordered_map<std::string_view, Value> m_map{};
};

class Environment::Enclosed : public Environment {
  public:
	Enclosed(Environment& enclosing) : enclosing(enclosing) {}

	bool assign(std::string_view const& key, Value value) override;
	Value const* find(std::string_view const& key) const override;
	Value* find(std::string_view const& key) override;

	Environment& enclosing;
};
} // namespace old

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

	std::vector<Page> m_stores{};
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
