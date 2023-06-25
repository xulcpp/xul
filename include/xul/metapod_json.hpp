#ifndef _xul_metapod_json_hpp_
#define _xul_metapod_json_hpp_
/// @file
/// Provides functions for converting metapods to JSON.

#include "metapod.hpp"

#include <string>
#include <string_view>


namespace xul {

/// Convert a metapod to JSON, with the assumption that all strings contain
/// only ASCII characters.
///
/// @todo This is a work in progress.
template <typename metapod>
std::string metapod_to_ascii_json(const metapod& pod)
{
  std::string json;
  var_for_each<typename metapod::xulmeta::xmp_fieldlist>([&json]<typename field>(){
    static constexpr std::string_view name_sv{field::xmp_name};
    json += '"';
    json += name_sv;
    json += "\":{},\n";
  });
  json.pop_back();
  return json;
}

}

#endif
