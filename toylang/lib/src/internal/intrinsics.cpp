#include <internal/intrinsics.hpp>
#include <toylang/interpreter.hpp>
#include <toylang/util.hpp>
#include <toylang/value.hpp>
#include <chrono>
#include <filesystem>

namespace toylang::intrinsics {
namespace fs = std::filesystem;

namespace {
bool check_arg_count(Interpreter& in, CallContext const& ctx, std::string_view name, std::size_t count) {
	if (ctx.args.size() != count) {
		auto msg = std::string{name};
		util::append(msg, " requires ", std::to_string(count), " argument(s)");
		in.runtime_error(ctx.callee, msg.c_str());
		return false;
	}
	return true;
}

template <typename T>
double to_double(T const& tp) {
	return std::chrono::duration<double>(tp.time_since_epoch()).count();
}
} // namespace

Value Print::operator()(Interpreter&, CallContext ctx) const {
	auto str = util::concat(ctx.args);
	util::append(str, "\n");
	util::print(str.c_str());
	return {.payload = static_cast<double>(ctx.args.size())};
}

Value PrintF::operator()(Interpreter& in, CallContext ctx) const {
	if (ctx.args.empty()) { return {.payload = 0.0}; }
	if (!ctx.args[0].contains<std::string>()) {
		in.runtime_error(ctx.callee, "printf: Invalid fmt");
		return {.payload = -1.0};
	}
	auto str = std::string{};
	auto ret = std::size_t{};
	std::string_view fmt = ctx.args[0].get<std::string>();
	ctx.args = ctx.args.subspan(1);
	while (!fmt.empty()) {
		if (auto const lbrace = fmt.find('{'); lbrace != std::string_view::npos) {
			auto const rbrace = fmt.find('}', lbrace);
			if (rbrace == std::string_view::npos) {
				in.runtime_error(ctx.callee, "printf: Unterminated '{'");
				return {.payload = -1.0};
			}
			util::append(str, fmt.substr(0, lbrace));
			if (!ctx.args.empty()) {
				util::append(str, to_string(ctx.args.front()));
				ctx.args = ctx.args.subspan(1);
				++ret;
			} else {
				util::append(str, "{}");
			}
			fmt = fmt.substr(rbrace + 1);
		} else {
			util::append(str, fmt);
			fmt = {};
		}
	}
	std::printf("%s", util::unescape(str).c_str());
	return {.payload = static_cast<double>(ret)};
}

Value Clone::operator()(Interpreter& in, CallContext ctx) const {
	if (!check_arg_count(in, ctx, name_v, 1)) { return {}; }
	auto const& ret = ctx.args.front();
	auto const visitor = Overloaded{
		[](StructInst const& si) {
			auto ret = StructInst{};
			ret.def = si.def;
			ret.fields = std::make_shared<StructInst::Fields>(*si.fields);
			return Value{std::move(ret)};
		},
		[](auto const& payload) { return Value{.payload = payload}; },
	};
	return ret.visit(visitor);
}

Value Str::operator()(Interpreter& in, CallContext ctx) const {
	if (!check_arg_count(in, ctx, name_v, 1)) { return {}; }
	return Value{.payload = to_string(ctx.args.front())};
}

Value Now::operator()(Interpreter& in, CallContext ctx) const {
	if (!check_arg_count(in, ctx, name_v, 0)) { return {}; }
	return Value{.payload = to_double(std::chrono::steady_clock::now())};
}

Value File::operator()(Interpreter& in, CallContext ctx) const {
	if (ctx.args.size() < 2) {
		in.runtime_error(ctx.callee, "_file: Requires at least two arguments");
		return {};
	}
	auto* op = std::get_if<std::string>(&ctx.args[0].payload);
	auto* arg0 = std::get_if<std::string>(&ctx.args[1].payload);
	if (!op || !arg0) {
		in.runtime_error(ctx.callee, "_file: Requires (string, string) arguments");
		return {};
	}
	if (*op == "read") { return Value{.payload = util::read_file(arg0->c_str())}; }
	if (*op == "write") {
		if (ctx.args.size() < 3) {
			in.runtime_error(ctx.callee, "_file.write: Requires (string, string, string) arguments");
			return {};
		}
		auto* arg1 = std::get_if<std::string>(&ctx.args[2].payload);
		if (!arg1) {
			in.runtime_error(ctx.callee, "_file.write: Invalid path");
			return {};
		}
		return Value{.payload = Bool{util::write_file(arg0->c_str(), *arg1)}};
	}
	if (*op == "remove") { return Value{.payload = Bool{fs::remove(*arg0)}}; }
	in.runtime_error(ctx.callee, "_file: Invalid operation");
	return {};
}
} // namespace toylang::intrinsics
