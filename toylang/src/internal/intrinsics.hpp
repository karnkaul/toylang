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

template <typename... T>
constexpr auto get_names() {
	return std::array{T::name_v...};
}

inline constexpr auto names_v = get_names<Print, PrintF, Clone>();

inline constexpr bool taken(std::string_view name) {
	for (auto const n : names_v) {
		if (name == n) { return true; }
	}
	return false;
}
} // namespace intrinsics
} // namespace toylang
