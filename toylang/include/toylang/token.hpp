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

	eEof,

	eCOUNT_
};
inline constexpr std::pair<TokenType, TokenType> keyword_range_v = {TokenType::eAnd, TokenType::eVar};

inline constexpr std::string_view token_str_v[] = {
	"plus",	   "minus", "star",	 "slash", "comma", "dot", "semicolon", "left_brace", "right_brace", "left_paren", "right_paren", "bang",
	"bang_eq", "eq",	"eq-eq", "gt",	  "ge",	   "lt",  "le",		   "identifier", "number",		"string",	  "and",		 "or",
	"true",	   "false", "fn",	 "for",	  "while", "if",  "else",	   "null",		 "return",		"this",		  "var",		 "eof",
};
static_assert(std::size(token_str_v) == static_cast<std::size_t>(TokenType::eCOUNT_));

inline constexpr TokenType increment(TokenType type) { return static_cast<TokenType>(static_cast<std::underlying_type_t<TokenType>>(type) + 1); }

struct Token {
	using Type = TokenType;

	std::string_view lexeme{};
	Location location{};
	TokenType type{TokenType::eEof};

	explicit constexpr operator bool() const { return type != TokenType::eEof; }
};
} // namespace toylang
