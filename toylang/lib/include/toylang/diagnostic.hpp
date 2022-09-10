#pragma once
#include <toylang/token.hpp>

namespace toylang {
struct Diagnostic {
	enum class Type : std::uint8_t { eRuntimeError, eSyntaxError, eInternalError, eWarning };

	struct Expect {
		TokenType type{TokenType::eEof};
		std::string_view text{};
	};

	static constexpr bool is_error(Type type) { return type < Type::eWarning; }

	bool is_error() const { return is_error(type); }

	Token token{};
	TokenType expected{TokenType::eEof};
	std::string_view message{};
	Type type{};
};
} // namespace toylang
