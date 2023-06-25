#ifndef _xul_metapod_hpp_
#define _xul_metapod_hpp_
/// @file
/// Defines macros and metafunctions for working wih Xul "metapods", which
/// are plain-old-data types that have compiletime (meta) data available, thanks
/// to the macros.
///
/// Being plain old data, these metapods are best used for data oriented tasks
/// like structured data for config and logging.

#include "macronomicon.hpp"
// For debugging what your metapod looks like, define this before including
// the header, and build with `g++ -E mymetapod.cpp | clang-format`
#ifndef XUL_METAPOD_DEBUG
#include "variadic.hpp"
#endif

/// Declares a POD, with the given _podname_, with fields defined by the
/// variadic arguments. It is easiest to refer to an example:
///
/// @code
/// xul_metapod(
///   person,
///   ((std::array<char, 5>), id),
///   ((std::string), name)
/// );
/// @endcode
///
/// Each field definition in the varidiac arguments is required to be a tuple
/// of:
/// * Parenthesis-enclosed type
/// * Name
///
/// The parentheses are required on the type to support type templates with
/// multiple parameters.
///
/// The above example generates a struct as follows:
/// @code
/// struct person final {
///   std::array<char, 5> id;
///   std::string name;
///   int type;
///   struct xulmeta;
/// };
/// @endcode
///
/// Note the forward declared `xulmeta` struct. The macro then immediately
/// defines the `xulmeta` struct to contain metadata for the POD, and each of
/// its fields, which captures:
/// * type
/// * index / position of member in POD
/// * name
/// * pointer to member
///
#define xul_metapod(podname, ...) \
struct podname final { \
  xul_mn_iter(xul_metapod___decl_fields, __VA_ARGS__) \
  struct xulmeta; \
}; \
struct podname::xulmeta final { \
  using xmp_pod = podname; \
  [[maybe_unused]] static constexpr const char* xmp_name{#podname}; \
  [[maybe_unused]] static constexpr unsigned xmp_field_count{xul_mn_argc(__VA_ARGS__)}; \
  struct xmp_fields final { \
    [[maybe_unused]] static constexpr unsigned xmp_count{xul_mn_argc(__VA_ARGS__)}; \
    xul_mn_nmrt(xul_metapod___decl_meta_field, __VA_ARGS__) \
  }; \
  using xmp_fieldlist = ::xul::tlist< \
    xmp_fields::xul_metapod___name_from_def(xul_mn_args_head(__VA_ARGS__)) \
    xul_mn_iter(xul_metapod___tlist_all_metafields, xul_mn_args_tail(__VA_ARGS__)) \
  >; \
}; \


/// Check whether or not _meta_ is a metapod, by looking for the presence of the
/// `xulmeta` field. This is purely for convenience of `static_assert`ing and
/// predicating any metapod metaprogramming on.
template <typename meta>
constexpr bool is_metapod_v{requires{typename meta::xulmeta;}};


// The below is implementation detail of the xul_metapod macros.

// Supplied with every field definition in a xul_metapod call when the fields of
// a metapod are being defined, i.e., defining the public data members of the
// POD.
#define xul_metapod___decl_fields(deftuple) xul_metapod___decl_fields_impl deftuple
#define xul_metapod___decl_fields_impl(type, name, ...) xul_mn_subst type name;

// Supplied with every field definition in a xul_metapod call when a 'metafield'
// is being defined, which provides compile constants for describing the field.
#define xul_metapod___decl_meta_field(i, def) xul_mn_apply(xul_metapod___decl_meta_field_impl, i, xul_mn_subst def)
#define xul_metapod___decl_meta_field_impl(findex, ftype, fname, ...) \
struct fname final { \
  using xmp_pod = xulmeta::xmp_pod; \
  using xmp_type = xul_mn_subst ftype; \
  using xmp_attrs = ::xul::tlist<__VA_ARGS__>; \
  [[maybe_unused]] static constexpr std::size_t xmp_index{findex}; \
  [[maybe_unused]] static constexpr const char* xmp_name{#fname}; \
  [[maybe_unused]] static constexpr auto xmp_ptr{&xmp_pod::fname}; \
};


// Takes a def tuple and gives only the name
#define xul_metapod___name_from_def(deftuple) xul_metapod___name_from_def_impl deftuple
#define xul_metapod___name_from_def_impl(type, name, ...) name

#define xul_metapod___tlist_all_metafields(deftuple) , xmp_fields::xul_metapod___name_from_def(deftuple)

#endif
