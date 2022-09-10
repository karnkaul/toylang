#pragma once
#include <toylang/literal.hpp>
#include <toylang/token.hpp>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <variant>

namespace toylang {
class Literal;
class Interpreter;
struct Value;

struct CallContext {
	Token callee{};
	std::span<Value> args{};
};

using Callback = std::function<Value(Interpreter&, CallContext)>;

///
/// \brief Function
///
struct Invocable {
	Token def{};
	Callback callback{};
};

struct StructInst;

///
/// \brief Struct definition
///
struct StructDef {
	std::string_view name{};
	std::vector<std::string_view> fields{};

	StructInst instance() const;
};

///
/// \brief Struct instance
///
struct StructInst {
	using Fields = std::unordered_map<std::string_view, Value>;

	StructDef def{};

	std::shared_ptr<Fields> fields{};
	Value const* find(std::string_view name) const;
	bool set(std::string_view name, Value&& value);
};

///
/// \brief Value
///
struct Value {
	using Payload = std::variant<std::nullptr_t, Bool, double, std::string, Invocable, StructDef, StructInst>;
	Payload payload{};

	static Value make(Literal const& literal);

	template <typename T>
	bool contains() const {
		return std::holds_alternative<T>(payload);
	}

	template <typename T>
	T& get() {
		return std::get<T>(payload);
	}

	template <typename T>
	T const& get() const {
		return std::get<T>(payload);
	}

	template <typename Visitor>
	decltype(auto) visit(Visitor&& v) {
		return std::visit(std::forward<Visitor>(v), payload);
	}

	template <typename Visitor>
	decltype(auto) visit(Visitor&& v) const {
		return std::visit(std::forward<Visitor>(v), payload);
	}

	bool is_null() const { return contains<std::nullptr_t>(); }
	bool is_bool() const { return contains<Bool>(); }

	bool is_truthy() const;

	std::string to_string() const;

	bool operator==(Value const& rhs) const;
};

template <typename... T>
struct Overloaded : T... {
	using T::operator()...;
};

template <typename... T>
Overloaded(T...) -> Overloaded<T...>;

std::string to_string(Value const& value);
} // namespace toylang
