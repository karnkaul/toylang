#pragma once
#include <array>
#include <string_view>

namespace toylang {
class Value;
class Interpreter;
struct CallContext;

namespace intrinsics {
struct Intrinsic {
	virtual ~Intrinsic() = default;

	virtual Value operator()(Interpreter& in, CallContext ctx) const = 0;
};

struct Print : Intrinsic {
	static constexpr std::string_view name_v = "_print";
	Value operator()(Interpreter& in, CallContext ctx) const override;
};

struct PrintF : Intrinsic {
	static constexpr std::string_view name_v = "_printf";
	Value operator()(Interpreter& in, CallContext ctx) const override;
};

struct Clone : Intrinsic {
	static constexpr std::string_view name_v = "_clone";
	Value operator()(Interpreter& in, CallContext ctx) const override;
};

struct Str : Intrinsic {
	static constexpr std::string_view name_v = "_str";
	Value operator()(Interpreter& in, CallContext ctx) const override;
};

struct Now : Intrinsic {
	static constexpr std::string_view name_v = "_now";
	Value operator()(Interpreter& in, CallContext ctx) const override;
};

struct File : Intrinsic {
	static constexpr std::string_view name_v = "_file";
	Value operator()(Interpreter& in, CallContext ctx) const override;
};
} // namespace intrinsics
} // namespace toylang
