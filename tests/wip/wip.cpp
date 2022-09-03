#include <toylang/expr.hpp>
#include <toylang/parser.hpp>
#include <toylang/scanner.hpp>
#include <iostream>
#include <string>

namespace toylang {
namespace {
void print_tokens(std::string_view line) {
	auto scanner = Scanner{line};
	while (auto token = scanner.next_token()) {
		// auto str = make_marker(line, token);
		// std::cout << str << '\n';
		std::cout << token.lexeme << " [" << detail::at(token_str_v, token.type) << "]\n";
	}
}

void print_ast(std::string_view line) {
	auto parser = Parser{line};
	while (auto expr = parser.parse()) {
		//
		std::cout << to_string(*expr) << '\n';
	}
}
} // namespace

void run() {
	auto cursor = [] { std::cout << ": "; };
	cursor();
	auto line = std::string{};
	while (std::getline(std::cin, line)) {
		if (line == "q" || line == "quit") { return; }
		// print_tokens(line);
		print_ast(line);
		cursor();
	}
}
} // namespace toylang

int main() {
	{
		auto five = std::make_unique<toylang::LiteralExpr>(5);
		auto two = std::make_unique<toylang::LiteralExpr>(2);
		auto three = std::make_unique<toylang::LiteralExpr>(3);
		auto two_minus_3 = std::make_unique<toylang::BinaryExpr>(std::move(two), toylang::Token{.lexeme = "-"}, std::move(three));
		auto five_plus_2_minus_3 = std::make_unique<toylang::BinaryExpr>(std::move(five), toylang::Token{.lexeme = "+"}, std::move(two_minus_3));
		std::cout << to_string(*five_plus_2_minus_3) << '\n';
	}
	toylang::run();
}
