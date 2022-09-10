#pragma once
#include <cstring>
#include <memory>
#include <span>
#include <string_view>

namespace toylang::util {
///
/// \brief Lightweight vector
///
template <typename Type>
class Buffer {
  public:
	Buffer() = default;
	Buffer(std::span<Type const> data) : m_data(std::make_unique_for_overwrite<Type[]>(data.size())), m_size(data.size()) {
		std::memcpy(m_data.get(), data.data(), data.size());
	}

	Type* data() const { return m_data.get(); }
	std::size_t size() const { return m_size; }
	std::span<Type> span() const { return {data(), size()}; }

  private:
	std::unique_ptr<Type[]> m_data{};
	std::size_t m_size{};
};

///
/// \brief Lightweight string
///
class CharBuf : public Buffer<char> {
  public:
	CharBuf(std::string_view str) : Buffer<char>{{str.data(), str.size()}} {}
	std::string_view view() const { return {data(), size()}; }
	operator std::string_view() const { return view(); }
};
} // namespace toylang::util
