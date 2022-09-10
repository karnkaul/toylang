#include <toylang/util.hpp>
#include <toylang/value.hpp>
#include <cassert>

namespace toylang {
namespace {
std::string from(double d) {
	auto const i = static_cast<std::int64_t>(d);
	if (static_cast<double>(i) == d) { return std::to_string(i); }
	return std::to_string(d);
}
} // namespace

StructInst StructDef::instance() const {
	auto ret = StructInst{*this};
	ret.fields = std::make_shared<StructInst::Fields>();
	for (auto const& field : fields) { ret.fields->insert_or_assign(field, Value{}); }
	return ret;
}

Value const* StructInst::find(std::string_view name) const {
	if (!fields) { return {}; }
	if (auto const it = fields->find(name); it != fields->end()) { return &it->second; }
	return {};
}

bool StructInst::set(std::string_view name, Value&& value) {
	if (!fields) { return false; }
	auto const it = fields->find(name);
	if (it == fields->end()) { return false; }
	it->second = std::move(value);
	return true;
}

Value Value::make(Literal const& literal) {
	auto ret = Value{};
	switch (literal.type()) {
	case Literal::Type::eString: ret.payload = util::unescape(literal.as_string()); break;
	case Literal::Type::eDouble: ret.payload = literal.as_double(); break;
	case Literal::Type::eBool: ret.payload = Bool{literal.as_bool()}; break;
	case Literal::Type::eNull: ret.payload = nullptr; break;
	default: assert(false && "Unexpected Literal::Type"); break;
	}
	return ret;
}

bool Value::is_truthy() const {
	static constexpr auto visitor = Overloaded{
		[](std::nullptr_t) { return false; },
		[](Bool b) { return b.value; },
		[](auto const&) { return true; },
	};
	return visit(visitor);
}

std::string Value::to_string() const { return toylang::to_string(*this); }

std::string to_string(Value const& value) {
	auto const visitor = Overloaded{
		[](std::nullptr_t) { return std::string{"null"}; },
		[](Bool const b) { return b ? std::string{"true"} : std::string{"false"}; },
		[](double const d) { return from(d); },
		[](std::string const& s) { return s; },
		[](Invocable const& i) { return std::string{"<fn " + std::string{i.def.lexeme} + ">"}; },
		[](StructDef const& s) { return std::string{s.name}; },
		[](StructInst const& s) { return std::string{s.def.name} + " instance"; },
	};
	return value.visit(visitor);
}

bool Value::operator==(Value const& rhs) const {
	auto const visitor = Overloaded{
		[&rhs](std::nullptr_t) { return rhs.is_null(); },
		[&rhs](Bool const b) {
			if (rhs.contains<std::string>()) { return false; }
			return b.value == rhs.is_truthy();
		},
		[&rhs](double const ld) {
			if (auto rd = std::get_if<double>(&rhs.payload)) { return ld == *rd; }
			if (rhs.contains<std::string>()) { return false; }
			return rhs.is_truthy();
		},
		[&rhs](std::string const& ls) {
			if (auto const& rs = std::get_if<std::string>(&rhs.payload)) { return ls == *rs; }
			return false;
		},
		[&rhs](Invocable const& li) {
			if (auto const& ri = std::get_if<Invocable>(&rhs.payload)) { return li.def.lexeme == ri->def.lexeme; }
			return false;
		},
		[&rhs](StructDef const& ls) {
			if (auto const& rs = std::get_if<StructDef>(&rhs.payload)) { return ls.name == rs->name; }
			return false;
		},
		[&rhs](StructInst const& li) {
			if (auto const& ri = std::get_if<StructInst>(&rhs.payload)) { return li.def.name == ri->def.name && li.fields.get() == ri->fields.get(); }
			return false;
		},
	};
	return visit(visitor);
}
} // namespace toylang
