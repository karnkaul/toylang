#pragma once
#include <cassert>
#include <concepts>
#include <cstring>
#include <string_view>

namespace toylang {
template <typename... T>
inline constexpr bool false_v = false;

struct Bool {
	bool value{};
	explicit constexpr operator bool() const { return value; }
	bool operator==(Bool const&) const = default;
	friend constexpr Bool operator!(Bool const& b) { return Bool{!b.value}; }
};

struct StringView {
	char const* data{};
	std::size_t size{};
};

class Literal {
  public:
	enum class Type { eNull, eBool, eDouble, eString };
	static constexpr char const* types_v[] = {"null", "bool", "double", "string"};

	static constexpr auto capacity_v = sizeof(StringView);
	static constexpr auto align_v = alignof(StringView);

	template <typename T>
	static constexpr Type type_for() {
		if constexpr (std::floating_point<T> || std::integral<T>) {
			return Type::eDouble;
		} else if constexpr (std::same_as<T, StringView>) {
			return Type::eString;
		} else {
			static_assert(false_v<T>, "Invalid type");
		}
	}

	template <typename T>
	static constexpr char const* type_str() {
		return type_str(type_for<T>());
	}

	static constexpr char const* type_str(Type type) { return types_v[static_cast<std::size_t>(type)]; }

	Literal() = default;
	Literal(bool) = delete;

	Literal(std::nullptr_t) {}
	Literal(Bool b) : Literal{Type::eBool, b} {}
	Literal(double const d) : Literal{Type::eDouble, d} {}
	Literal(StringView const s) : Literal{Type::eString, s} {}

	template <std::integral T>
	Literal(T const t) : Literal{static_cast<double>(t)} {}
	Literal(std::string_view const s) : Literal{StringView{s.data(), s.size()}} {}

	Type type() const { return m_type; }
	explicit operator bool() const { return m_type != Type::eNull; }

	Bool as_bool() const {
		assert(type() == Type::eBool);
		return *reinterpret_cast<Bool const*>(m_buffer);
	}

	double as_double() const {
		assert(type() == Type::eDouble);
		return *reinterpret_cast<double const*>(m_buffer);
	}

	std::string_view as_string() const {
		assert(type() == Type::eString);
		auto const ret = reinterpret_cast<StringView const*>(m_buffer);
		return {ret->data, ret->size};
	}

  private:
	template <typename T>
	Literal(Type type, T const t) : m_type{type} {
		static_assert(sizeof(T) <= capacity_v);
		std::memcpy(m_buffer, &t, sizeof(T));
	}

	alignas(align_v) std::byte m_buffer[capacity_v]{};
	Type m_type{Type::eNull};
};
} // namespace toylang
