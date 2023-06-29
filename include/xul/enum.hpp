#ifndef _xul_enum_hpp_
#define _xul_enum_hpp_
/// @file
/// Xul's enum wrapper, powered by the macronomicon.

#include "macronomicon.hpp"

// For debugging what your xenum looks like, define this before including
// the header, and build with `g++ -E myxenum.cpp | clang-format`
#ifndef XUL_ENUM_DEBUG
#include <optional>
#include <string_view>
#include <type_traits>
#include <initializer_list>
#endif

namespace xul {

/// Define a linear/contiguous xul enum. Xul enums encapsulate a plain C++ enum
/// but offer additional capabilities often required when dealing with enums.
///
/// # Safer construction
///
/// Xul enums are not default constructible. They must be constructed from one
/// of the enumerators. This prevents this kind of error:
///
/// ```{.cpp}
/// enum foo { a = 1, b, c};
/// foo f{};
/// // value of f? It's not 1. It's 0
/// ```
///
/// # Improvements
///
/// Xenums cannot currently be extended. A simple change to the macro is needed
/// to allow it to be used within a user defined struct:
/// ```{.cpp}
/// struct MyXenum : public ::xul::enum::tag {
///   xul_i_am_become_xenum(MyXenum, A, B, C);
/// public:
///   constexpr bool isForCookie() const { return value_ == C; }
/// };
/// ```
///
#define xul_enum(name, underlying_type, ...) \
struct name : public ::xul::xenum::tag \
{ \
  /* The encapsulated plain C++ enum is called `xenum`. It will have an
     underlying type if the macro argument is not empty.*/ \
  enum class xenum \
    xul_mn_if_non_empty(underlying_type, : underlying_type) \
  { \
    xul_enum___decl_xenum(__VA_ARGS__) \
  }; \
\
  /* The enum is `used` so that it the enumerators can be accessed via the
     the xul enum wrapper. */ \
  using enum xenum; \
\
  /* Creating a xul enum from the wrapped enum is implictly allowed. */ \
  constexpr name(xenum val) noexcept : value_{val} {} \
  constexpr name(const name&) noexcept = default; \
\
  /* Friend factory function that allows for creation of the xul enum wrapper
     via ADL from one of the internal xenum enumerators, which would otherwise
     casue the type to be deduced as the internal xenum. */ \
  [[maybe_unused]] constexpr friend name mk(xenum x) noexcept { \
    return name{x}; \
  } \
\
  /* Factory function for attempting to create an enum from some integral type.
     If value _t_ is not in the range of enumerators, std::nullopt is returned.*/ \
  template <typename T> \
  static constexpr std::optional<name> try_mk(const T t) noexcept \
  { \
    using ult = std::underlying_type_t<xenum>; \
    if ( t < static_cast<ult>(first().value_) ) return std::nullopt; \
    if ( t > static_cast<ult>(last().value_) ) return std::nullopt; \
    return name{static_cast<xenum>(t)}; \
  }  \
\
  /* The enum wrapper is implicitly convertible to its encapsulated plain C++
     enum. This allows the xul enum wrapper to still work in switch statements. */ \
  [[maybe_unused]] constexpr operator xenum() const { \
    return value_; \
  } \
  \
  /* Get the held xenum as its raw underlying type */ \
  [[maybe_unused]] constexpr std::underlying_type_t<xenum> to_ult() const { \
    return static_cast<std::underlying_type_t<xenum>>(value_); \
  } \
\
  /* Convenience version for static_casting the held enumerator */ \
  template <typename T> \
  constexpr T to() const { \
    return static_cast<T>(value_); \
  } \
\
  /* Get the stringified version of an enumerator. Since this wrapper class is
    implictly convertible, it works with the xenum enumerators. */ \
  [[maybe_unused]] constexpr friend std::string_view to_string(const name e) { \
    return name{e}.to_string(); \
  } \
\
  /* Get the stringified version of the currently held enumerator */ \
  [[maybe_unused]] constexpr std::string_view to_string() const { \
    switch ( value_ ) { \
      xul_enum___to_string(__VA_ARGS__) \
    } \
    return "?" #name "?"; \
  } \
\
  /* Get the count of enumerators in this Xul enum */ \
  [[maybe_unused]] static constexpr unsigned count() noexcept { \
    return xul_mn_argc(__VA_ARGS__); \
  } \
\
  /* Get the first enumerator in this Xul enum */ \
  [[maybe_unused]] static constexpr name first() noexcept { \
    return ::xul::xenum::first({xul_enum___just_names(__VA_ARGS__)}); \
  } \
\
  /* Get the last enumerator in this Xul enum */ \
  [[maybe_unused]] static constexpr name last() noexcept { \
    return ::xul::xenum::last({xul_enum___just_names(__VA_ARGS__)}); \
  } \
\
  [[maybe_unused]] static constexpr ::xul::xenum::range<name> range( \
    xenum begin = first(), \
    xenum end = last()) { return {begin, end}; } \
\
  [[maybe_unused]] consteval friend std::type_identity<name> id( \
    std::type_identity<name>) noexcept { return {}; } \
\
  [[maybe_unused]] consteval friend std::type_identity<name> id( \
    std::type_identity<xenum>) noexcept { return {}; } \
\
private: \
  xenum value_; \
}


/// This holds all the non preprocessor stuff that xenums use, such as tags,
/// iterators, ranges, etc.
struct xenum final
{
  struct tag {};

  template <typename E>
  static consteval E first(const std::initializer_list<E> es) {
    using I = std::underlying_type_t<E>;
    auto it = es.begin();
    I f{static_cast<I>(*it)};
    ++it;
    while (it != es.end()) {
      f = std::min<I>(f, static_cast<I>(*it));
      ++it;
    }
    return static_cast<E>(f);
  }

  template <typename E>
  static consteval E last(const std::initializer_list<E> es) {
    using I = std::underlying_type_t<E>;
    auto it = es.begin();
    I f{static_cast<I>(*it)};
    ++it;
    while (it != es.end()) {
      f = std::max<I>(f, static_cast<I>(*it));
      ++it;
    }
    return static_cast<E>(f);
  }


  template <typename E>
    requires(std::is_base_of_v<tag, E>)
  struct end_iter
  {
    using ult = std::underlying_type_t<typename E::xenum>;

    constexpr end_iter(E stop) noexcept : stop{stop.to_ult() + 1} {}

    const ult stop;
  };

  template <typename E>
    requires(std::is_base_of_v<tag, E>)
  struct iter
  {
    using ult = std::underlying_type_t<typename E::xenum>;

    constexpr iter(E start) noexcept : value_{start.to_ult()} {}
    constexpr iter(const iter&) noexcept = default;

    constexpr E operator*() const noexcept {
      return E{static_cast<typename E::xenum>(value_)};
    }

    constexpr bool operator !=(const end_iter<E>& other) noexcept {
      return value_ != other.stop;
    }

    constexpr iter& operator++() noexcept{
      ++value_;
      return *this;
    }

  private:
    ult value_;
  };


  template <typename E>
  struct range
  {
    const E first;
    const E last;

    constexpr range(E first = E::first(), E last = E::last()) noexcept
      : first{first}, last{last} {}

    constexpr iter<E> begin() const noexcept {
      return {first};
    }

    constexpr end_iter<E> end() const noexcept {
      return {last};
    }
  };

  template <typename E>
  range(E first = E::first(), E last = E::last()) -> range<typename decltype(id(std::type_identity<E>{}))::type>;
};


//
// Implementation details of the xul_enum macro
//

#define xul_enum___is_pair(p) \
  xul_mn_apply(xul_enum___is_pair_2, xul_mn_subst p, 1, 0)
#define xul_enum___is_pair_2(one, two, result, ...) result


#define xul_enum___decl_xenum(...) \
   xul_mn_iter(xul_enum___decl_xenum2, __VA_ARGS__)
#define xul_enum___decl_xenum2(maybe_pair) \
  xul_mn_if_else( \
    xul_enum___is_pair(maybe_pair), \
    xul_enum___decl_xenum_pair, \
    xul_enum___decl_xenum_single \
  )(maybe_pair)
#define xul_enum___decl_xenum_single(x) x,
#define xul_enum___decl_xenum_pair(pair) xul_enum___decl_xenum_pair2 pair
#define xul_enum___decl_xenum_pair2(name, val) name = val,


#define xul_enum___to_string(...) \
  xul_mn_iter(xul_enum___to_string2, __VA_ARGS__)
#define xul_enum___to_string2(maybe_pair) \
  xul_mn_if_else( \
    xul_enum___is_pair(maybe_pair), \
    xul_enum___to_string_pair, \
    xul_enum___to_string_single \
  )(maybe_pair)
#define xul_enum___to_string_single(e) case e: return #e;
#define xul_enum___to_string_pair(pair) xul_enum___to_string_pair2 pair
#define xul_enum___to_string_pair2(e, val) xul_enum___to_string_single(e)


#define xul_enum___just_names(...) \
   xul_mn_iter(xul_enum___just_names2, __VA_ARGS__)
#define xul_enum___just_names2(maybe_pair) \
  xul_mn_if_else( \
    xul_enum___is_pair(maybe_pair), \
    xul_enum___just_names_pair, \
    xul_enum___just_names_single \
  )(maybe_pair)
#define xul_enum___just_names_single(name) name,
#define xul_enum___just_names_pair(pair) xul_enum___just_names_pair2 pair
#define xul_enum___just_names_pair2(name, val) name,

}

#endif
