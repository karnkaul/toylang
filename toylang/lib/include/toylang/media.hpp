#pragma once
#include <string>
#include <vector>

namespace toylang {
struct Media {
	std::vector<std::string> mounted{};

	bool mount(std::string_view path);
	bool is_mounted(std::string_view path) const;
	bool exists(std::string_view uri) const;
	bool read_to(std::string& out, std::string_view uri) const;
};
} // namespace toylang
