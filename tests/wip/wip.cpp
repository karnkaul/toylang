#include <toylang/interpreter.hpp>
#include <toylang/scanner.hpp>
#include <toylang/util/reporter.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace {
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
		int index{};
		for (int i = 1; i < argc; ++i) {
			auto const arg = std::string_view{argv[i]};
			if (arg[0] == '-') {
				index = add_options(index, arg.substr(1));
			} else {
				break;
			}
		}
		args = {argv + index, static_cast<std::size_t>(argc)};
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

std::string file_str(char const* path) {
	if (auto file = std::ifstream(path, std::ios::binary | std::ios::ate)) {
		auto const size = file.tellg();
		file.seekg({});
		auto ret = std::string(static_cast<std::size_t>(size), '\0');
		file.read(ret.data(), size);
		return ret;
	}
	return {};
}

namespace tl = toylang;

struct Repl {
	tl::Interpreter interpreter;
	char cursor = '>';

	Repl(tl::Interpreter::Debug flags = {}, std::unique_ptr<tl::util::Notifier> custom = {}) : interpreter{std::move(custom)} { interpreter.debug = flags; }

	bool execute(std::string_view line) {
		auto ret = interpreter.execute_or_evaluate({.text = line});
		return ret;
	}

	void run() {
		auto write_cursor = [c = cursor] { std::cout << c << " "; };
		write_cursor();
		auto line = std::string{};
		while (std::getline(std::cin, line)) {
			if (line == "q" || line == "quit") { return; }
			execute(line);
			write_cursor();
		}
	}
};
} // namespace

int main(int argc, char** argv) {
	auto const exe_name = std::filesystem::path(argv[0]).filename().generic_string();
	auto args = CmdArgs{argc, argv};
	if (args.option("help")) {
		std::cout << "Usage: " << exe_name << " [path/to/script] [--options]\n\n";
		std::cout << "OPTIONS\n\n[ --verbose | -v ] \tPrint lots of debug text\n";
		return EXIT_SUCCESS;
	}
	auto debug_flags = tl::Interpreter::Debug{};
	if (args.option("verbose", 'v')) {
		debug_flags |= tl::Interpreter::ePrintStmtExprs;
		std::cout << "[Debug] Verbose mode enabled\n";
	}
	auto repl = Repl{debug_flags};
	repl.run();
}
