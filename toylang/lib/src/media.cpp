#include <toylang/media.hpp>
#include <toylang/util.hpp>
#include <algorithm>
#include <filesystem>

namespace toylang {
namespace fs = std::filesystem;

namespace {

template <typename T, typename F>
bool readf(std::span<std::string const> mounted, std::string_view uri, F f) {
	for (auto const& prefix : mounted) {
		auto const path = fs::path{prefix} / uri;
		if (fs::is_regular_file(path)) {
			f(path);
			return true;
		}
	}
	auto const path = fs::path{uri};
	if (fs::is_regular_file(path)) {
		f(path);
		return true;
	}
	return false;
}
} // namespace

bool Media::mount(std::string_view path) {
	auto abs = fs::absolute(path);
	if (std::find(mounted.begin(), mounted.end(), abs.generic_string()) != mounted.end()) { return true; }
	if (fs::is_directory(abs)) {
		mounted.push_back(abs.generic_string());
		return true;
	}
	return false;
}

bool Media::is_mounted(std::string_view path) const { return std::find(mounted.begin(), mounted.end(), path) != mounted.end(); }

bool Media::exists(std::string_view uri) const {
	return readf<bool>(mounted, uri, [](fs::path const&) {});
}

bool Media::read_to(std::string& out, std::string_view uri) const {
	return readf<std::string>(mounted, uri, [&out](fs::path const& path) { out = util::read_file(path.string().c_str()); });
}
} // namespace toylang
