#include <internal/intrinsics.hpp>
#include <toylang/interpreter.hpp>
#include <toylang/util.hpp>
#include <toylang/value.hpp>

namespace toylang::intrinsics {
Value Print::operator()(Interpreter&, CallContext ctx) const {
	auto str = util::concat(ctx.args);
	util::append(str, "\n");
	util::print_unescaped(str);
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
	if (ctx.args.empty()) {
		in.runtime_error(ctx.callee, "clone: Required argument missing");
		return {};
	}
	if (ctx.args.size() > 1) {
		in.runtime_error(ctx.callee, "clone: Single argument required");
		return {};
	}
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
} // namespace toylang::intrinsics
