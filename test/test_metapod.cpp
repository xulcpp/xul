// Perform `g++ -E -DINSPECT test_metapod.cpp | clang-format` for example output
#ifdef INSPECT
  #define XUL_METAPOD_DEBUG
  #include "../include/xul/metapod.hpp"
#else
  #include <xul/metapod.hpp>
  #include <array>
  #include <string>
  #include <string_view>
#endif

namespace {

using namespace std::literals;

xul_metapod(
  Person,
  ((std::array<char, 5>), id),
  ((std::string), name)
);

// These tests also show example usage, where you

// Check you're dealing with a metapod using the helper-for-the-lazy
static_assert(is_metapod_v<Person>);

// Alias and access it's metadata
using Meta = Person::xulmeta;
static_assert(Meta::xmp_name == "Person"sv);
static_assert(Meta::xmp_field_count == 2);

// Alias its fields
using Fields = Meta::xmp_fields;
// Access field metadata
static_assert(Fields::xmp_count == 2); // Note count available at meta and fields level

// `id` field of Person specific things
// Owning pod type is available from any field, in case you're doing crazy tmp.
static_assert(std::is_same_v<Fields::id::xmp_pod, Person>);
static_assert(std::is_same_v<Fields::id::xmp_type, std::array<char, 5>>);
static_assert(Fields::id::xmp_index == 0);
static_assert(Fields::id::xmp_name == "id"sv);
static_assert(Fields::id::xmp_ptr == &Person::id);

}
