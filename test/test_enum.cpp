// Perform `g++ -E -DINSPECT test_enum.cpp | clang-format` for example output
#ifdef INSPECT
  #define XUL_ENUM_DEBUG
  #include "../include/xul/enum.hpp"
#else
  #include <xul/enum.hpp>

  #include <gtest/gtest.h>
#endif


namespace {

xul_enum(Foo, int, (apple, 10), banana, carrot);

static_assert(Foo::count() == 3);
static_assert(Foo::first() == Foo::apple);
static_assert(Foo::last() == Foo::carrot);
static_assert(Foo::try_mk(-1) == std::nullopt);
static_assert(Foo::try_mk(10) == Foo::apple);
static_assert(Foo::try_mk(12) == Foo::carrot);
static_assert(Foo::try_mk(9) == std::nullopt);
static_assert(Foo::try_mk(13) == std::nullopt);


constexpr auto x = mk(Foo::banana);
static_assert(std::is_same_v<decltype(x), const Foo>);
constexpr auto y = Foo::carrot;
static_assert(std::is_same_v<decltype(y), const Foo::xenum>);

TEST(Xenum, Iterate)
{
  int count{};
  for ( auto f : Foo::range() ) {
    ++count;
    switch (f) {
    case Foo::apple:  EXPECT_EQ(f.to_string(), "apple");  break;
    case Foo::banana: EXPECT_EQ(f.to_string(), "banana"); break;
    case Foo::carrot: EXPECT_EQ(f.to_string(), "carrot"); break;
    }
  }
  EXPECT_EQ(count, Foo::count());
}

}
