#include <cmd_args.hpp>
#include <toylang/interpreter.hpp>
#include <toylang/util.hpp>
#include <filesystem>
#include <iostream>

namespace toylang {
namespace fs = std::filesystem;

namespace {
struct Runner {
	Interpreter interpreter{};

	bool execute(Source script) { return interpreter.execute_or_evaluate(script); }

	bool open(char const* path) {
		auto text = util::read_file(path);
		if (text.empty()) {
			std::cerr << "Failed to open " << path << '\n';
			return false;
		}
		return execute({.filename = path, .text = text});
	}

	void run(std::string_view cursor = ">") {
		auto write_cursor = [c = cursor] { std::cout << c << " "; };
		write_cursor();
		auto line = std::string{};
		while (std::getline(std::cin, line)) {
			if (line == "q" || line == "quit") { return; }
			execute({.text = line});
			write_cursor();
		}
	}
};

int run(int argc, char const* const argv[]) {
	auto const exe_path = fs::absolute(argv[0]);
	auto const exe_name = exe_path.filename().generic_string();
	auto const stdlib_path = [&] {
		for (fs::path path = exe_path; !path.empty() && path.parent_path() != path; path = path.parent_path()) {
			auto ret = path / "stdlib";
			if (fs::is_regular_file(ret / "std.tl")) { return ret.generic_string(); }
			ret = path / "toylang/stdlib";
			if (fs::is_regular_file(ret / "std.tl")) { return ret.generic_string(); }
		}
		return std::string{};
	}();
	auto args = CmdArgs{argc, argv};
	if (args.option("help")) {
		std::cout << "Usage: " << exe_name << " [path/to/script] [--options]\n\n";
		std::cout << "OPTIONS\n\n[ --verbose | -v ] \tPrint lots of debug text\n";
		return EXIT_SUCCESS;
	}
	auto debug_flags = toylang::Interpreter::Debug{};
	if (args.option("verbose", 'v')) {
		debug_flags |= toylang::Interpreter::ePrintStmtExprs;
		std::cout << "[Debug] Verbose mode enabled\n";
	}
	auto runner = toylang::Runner{};
	runner.interpreter.debug = debug_flags;
	runner.interpreter.media.mount(exe_path.parent_path().generic_string());
	if (!stdlib_path.empty()) {
		runner.interpreter.media.mount(stdlib_path);
		runner.interpreter.execute({.text = R"(import "std.tl";)"});
	}
	if (args.args.empty()) {
		runner.run();
	} else {
		if (!runner.open(args.args.front())) { return EXIT_FAILURE; }
	}
	return EXIT_SUCCESS;
}
} // namespace
} // namespace toylang

int main(int argc, char** argv) { return toylang::run(argc, argv); }
