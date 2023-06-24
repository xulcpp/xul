#include <xul/variadic.hpp>

namespace {
using namespace xul;

static_assert(tlist_size_v<tlist<int, char, bool>> == 3);
static_assert(vlist_size_v<vlist<10, 'a', 3.14>> == 3);

static_assert(var_size_v<tlist<int, char, bool>> == 3);
static_assert(var_size_v<vlist<10, 'a', 3.14>> == 3);

static_assert(std::is_same_v<
  typename tlist_left<2, tlist<int, char, bool>>::type,
  tlist<int, char>
>);

static_assert(std::is_same_v<
  tlist_right_t<2, tlist<int, bool, char>>,
  tlist<bool, char>
>);

void foo()
{
    var_for_each<tlist<int, char, bool>>([]<typename T>(){
        constexpr bool isInt{std::is_same_v<T, int>};
        constexpr bool isChar{std::is_same_v<T, char>};
        constexpr bool isBool{std::is_same_v<T, bool>};

        static_assert(isInt || isChar || isBool);
    });
}



}
