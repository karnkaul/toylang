#pragma once
#include <span>
#include <string_view>

namespace toylang {
struct CmdArgs {
	struct Option {
		std::string_view key{};

		explicit constexpr operator bool() const { return !key.empty(); }
	};

	static constexpr std::size_t max_options_v{8};

	Option options[max_options_v]{};
	std::span<char const* const> args{};

	static constexpr bool is_option(std::string_view const arg) { return arg[0] == '-'; }

	constexpr CmdArgs(int argc, char const* const* argv) {
		int index{1};
		for (; index < argc; ++index) {
			auto const arg = std::string_view{argv[index]};
			if (arg[0] == '-') {
				index = add_options(index, arg.substr(1));
			} else {
				break;
			}
		}
		args = {argv + index, static_cast<std::size_t>(argc - 1)};
	}

	constexpr int add_options(int index, std::string_view arg) {
		if (arg.empty()) { return index; }
		if (arg[0] == '-') {
			options[index++] = {arg.substr(1)};
		} else {
			for (char const& ch : arg) { options[index++] = {std::string_view{&ch, 1}}; }
		}
		return index;
	}

	constexpr Option option(std::string_view full, char single = '\0') const {
		for (auto const& option : options) {
			if (!option) { break; }
			if (option.key == full || (single != '\0' && option.key[0] == single)) { return option; }
		}
		return {};
	}
};
} // namespace toylang
