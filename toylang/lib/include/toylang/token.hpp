#pragma once
#include <toylang/location.hpp>
#include <string_view>

namespace toylang {
enum class TokenType {
	ePlus,
	eMinus,
	eStar,
	eSlash,
	eComma,
	eDot,
	eSemicolon,
	eBraceL,
	eBraceR,
	eParenL,
	eParenR,

	eBang,
	eBangEq,
	eEq,
	eEqEq,
	eGt,
	eGe,
	eLt,
	eLe,

	eIdentifier,
	eNumber,
	eString,

	eAnd,
	eOr,
	eTrue,
	eFalse,
	eFn,
	eFor,
	eWhile,
	eIf,
	eElse,
	eNull,
	eReturn,
	eThis,
	eVar,
	eBreak,
	eStruct,
	eImport,

	eEof,

	eCOUNT_
};
inline constexpr std::pair<TokenType, TokenType> single_range_v = {TokenType::ePlus, TokenType::eBang};
inline constexpr std::pair<TokenType, TokenType> keyword_range_v = {TokenType::eAnd, TokenType::eEof};

inline constexpr std::string_view token_str_v[] = {
	"+",  "-",	 "*",	  "/",	",",	".",	";",		  "{",		"}",	  "(",	   ")",		 "!",	   "!=",
	"=",  "==",	 ">",	  ">=", "<",	"<=",	"identifier", "number", "string", "and",   "or",	 "true",   "false",
	"fn", "for", "while", "if", "else", "null", "return",	  "this",	"var",	  "break", "struct", "import", "eof",
};
static_assert(std::size(token_str_v) == static_cast<std::size_t>(TokenType::eCOUNT_));

namespace detail {
template <typename Enum, typename Cont, typename T>
constexpr auto at(Cont const& container, Enum const e, T const& fallback = {}) {
	auto const index = static_cast<std::size_t>(e);
	if (index >= std::size(container)) { return fallback; }
	return container[index];
}
} // namespace detail

inline constexpr std::string_view token_string(TokenType const type, std::string_view const fallback = "[unknown]") {
	return detail::at(token_str_v, type, fallback);
}

inline constexpr TokenType increment(TokenType type) { return static_cast<TokenType>(static_cast<std::underlying_type_t<TokenType>>(type) + 1); }

struct Token {
	using Type = TokenType;

	std::string_view lexeme{};
	Location location{};
	TokenType type{TokenType::eEof};

	explicit constexpr operator bool() const { return type != TokenType::eEof; }
};
} // namespace toylang
