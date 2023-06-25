#include <xul/fnv_hash.hpp>

#include <string_view>

namespace {
using namespace xul;

constexpr bool test(std::string_view vec, const std::uint32_t u32, const std::uint64_t u64)
{
  return
    fnv1a_32(vec) == u32 &&
    fnv1a_64(vec) == u64;
}

// Sample of test vectors from the original code
static_assert(test("",       0x811c9dc5, 0xcbf29ce484222325));
static_assert(test("a",      0xe40c292c, 0xaf63dc4c8601ec8c));
static_assert(test("b",      0xe70c2de5, 0xaf63df4c8601f1a5));
static_assert(test("c",      0xe60c2c52, 0xaf63de4c8601eff2));
static_assert(test("d",      0xe10c2473, 0xaf63d94c8601e773));
static_assert(test("e",      0xe00c22e0, 0xaf63d84c8601e5c0));
static_assert(test("f",      0xe30c2799, 0xaf63db4c8601ead9));
static_assert(test("fo",     0x6222e842, 0x08985907b541d342));
static_assert(test("foo",    0xa9f37ed7, 0xdcb27518fed9d577));
static_assert(test("foob",   0x3f5076ef, 0xdd120e790c2512af));
static_assert(test("fooba",  0x39aaa18a, 0xcac165afa2fef40a));
static_assert(test("foobar", 0xbf9cf968, 0x85944171f73967e8));

}
