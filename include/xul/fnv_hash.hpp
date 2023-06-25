#ifndef _xul_fnv_hash_hpp_
#define _xul_fnv_hash_hpp_

#include <array>
#include <cstdint>
#include <span>
#include <string_view>

namespace xul {

template <typename H, typename T>
constexpr H fnv1a(const std::span<const T> data)
{
  constexpr auto prime{sizeof(H) == 4 ? 0x01000193 : 0x00000100000001b3};
  constexpr auto basis{sizeof(H) == 4 ? 0x811c9dc5 : 0xcbf29ce484222325};

  H hash{basis};
  for ( auto b : data ) {
    hash ^= b;
    hash *= prime;
  }
  return hash;
}

template <typename H, std::size_t N>
constexpr H fnv1a(const char(&literal)[N])
{
  return fnv1a<H>(std::span{literal, N-1});
}

template <typename H>
constexpr H fnv1a(const std::string_view str)
{
  return fnv1a<H>(std::span{str.data(), str.size()});
}

constexpr std::uint32_t fnv1a_32(const auto& data) { return fnv1a<std::uint32_t>(data); }
constexpr std::uint64_t fnv1a_64(const auto& data) { return fnv1a<std::uint64_t>(data); }

}

#endif
