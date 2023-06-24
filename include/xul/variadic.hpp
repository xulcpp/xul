#ifndef _xul_variadic_hpp_
#define _xul_variadic_hpp_

#include <type_traits>

namespace xul {

// tlist

/// Metaprogramming type for capturing a list of type template parameters.
/// Capturing type lists allows for partial specialisation and tag dispatch on
/// a set of types.
template <typename...>
struct tlist {};

template <typename...>
struct tlist_size;

template <template <typename...> typename tlist, typename... ts>
struct tlist_size<tlist<ts...>> {
  static constexpr auto value{sizeof...(ts)};
};


/// Get a typelist containing only the _n_ leftmost elements of the type list.
template <unsigned n, typename...>
struct tlist_left;

template <unsigned n, template <typename...> typename tlist, typename... ts>
struct tlist_left<n, tlist<ts...>>
{
  template <unsigned i, typename u, typename... us, typename... vs>
  static consteval auto mk(tlist<vs...> x) {
    if constexpr (i == 0) {
      return x;
    } else {
      return mk<i - 1, us...>(tlist<vs..., u>{});
    }
  }

  using type = decltype(mk<n, ts...>(tlist<>{}));
};

template <unsigned n, typename tlist>
using tlist_left_t = typename tlist_left<n, tlist>::type;

template <unsigned n, typename ...>
struct tlist_right;

template <unsigned n, template <typename...> typename tlist, typename... ts>
struct tlist_right<n, tlist<ts...>>
{
  template <unsigned i, typename u, typename... us>
  static consteval auto mk() {
    if constexpr ( i == 0 ) {
      return mk<i - 1, us...>();
    } else {
      return tlist<us...>{};
    }
  }

  using type = decltype(mk<sizeof...(ts) - n, ts...>());
};

template <unsigned n, typename tlist>
using tlist_right_t = typename tlist_right<n, tlist>::type;


/// Determine the size of a typelist, i.e., how many types are in the typelist.
/// This happens to work with any variadic type, such as std::variant or
/// std::tuple, not just xul's typelist.
///
/// ~~~{.cpp}
/// static_assert(tlist_size_v<tlist<int, char, bool, double>> == 4);
/// ~~~
template <typename tlist>
constexpr std::size_t tlist_size_v{tlist_size<tlist>::value};



// vlist

/// Metaprogramming type for capturing a list of non-type template parameters,
/// AKA values.
template <auto...>
struct vlist{};

template <typename>
struct vlist_size;

template <template <auto...> typename vlist, auto... vals>
struct vlist_size<vlist<vals...>> {
  static constexpr std::size_t value{sizeof...(vals)};
};

template <typename vlist>
constexpr std::size_t vlist_size_v{vlist_size<vlist>::value};



// var: works on type lists and value lists.

template <typename... >
struct var_size;

template <template <typename...> typename tlist, typename... ts>
struct var_size<tlist<ts...>> {
  static constexpr std::size_t value{sizeof...(ts)};
};

template <template <auto...> typename vlist, auto... vals>
struct var_size<vlist<vals...>> {
  static constexpr std::size_t value{sizeof...(vals)};
};

/// Determines the size of a typelist or valuelist.
template <typename x>
constexpr std::size_t var_size_v{var_size<x>::value};

template <typename...>
struct var_for_each_t;

template <template <typename...> typename tlist, typename... ts>
struct var_for_each_t<tlist<ts...>>
{
  template <typename fn>
  constexpr void operator()(fn&& f) const {
    (f.template operator()<ts>(), ...);
  }
};

template <template <auto...> typename vlist, auto... vals>
struct var_for_each_t<vlist<vals...>>
{
  template <typename fn>
  constexpr void operator()(fn&& f) const {
    (f.template operator()<vals>(), ...);
  }
};

/// Compiletime iterate a typelist or value list, by suppying an invocable with
/// a templated call operator.
template <typename list, typename fn>
constexpr void var_for_each(fn&& f)
{
  var_for_each_t<list>{}(f);
}


}

#endif
