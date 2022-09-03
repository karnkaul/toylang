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
};

struct StringView {
	char const* data{};
	std::size_t size{};
};

class Trivial {
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

	Trivial() = default;
	Trivial(bool) = delete;

	Trivial(std::nullptr_t) {}
	Trivial(Bool b) : Trivial{Type::eBool, b} {}
	Trivial(double const d) : Trivial{Type::eDouble, d} {}
	Trivial(StringView const s) : Trivial{Type::eString, s} {}

	template <std::integral T>
	Trivial(T const t) : Trivial{static_cast<double>(t)} {}
	Trivial(std::string_view const s) : Trivial{StringView{s.data(), s.size()}} {}

	Type type() const { return m_type; }
	explicit operator bool() const { return m_type != Type::eNull; }

	bool as_bool() const {
		assert(type() == Type::eDouble);
		auto const ret = reinterpret_cast<Bool const*>(m_buffer);
		return ret->value;
	}

	double as_double() const {
		assert(type() == Type::eDouble);
		auto const ret = reinterpret_cast<double const*>(m_buffer);
		return *ret;
	}

	std::string_view as_string() const {
		assert(type() == Type::eString);
		auto const ret = reinterpret_cast<StringView const*>(m_buffer);
		return {ret->data, ret->size};
	}

  private:
	template <typename T>
	Trivial(Type type, T const t) : m_type{type} {
		static_assert(sizeof(T) <= capacity_v);
		std::memcpy(m_buffer, &t, sizeof(T));
	}

	alignas(align_v) std::byte m_buffer[capacity_v]{};
	Type m_type{Type::eNull};
};
} // namespace toylang
