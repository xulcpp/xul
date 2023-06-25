#include <xul/metapod_json.hpp>

#include <gtest/gtest.h>

namespace {
using namespace xul;

xul_metapod(
   Fixcha,
   ((int), one),
   ((std::vector<char>), text)
);

TEST(MetapodJson, ToAsciiJson)
{
  Fixcha f;
  const auto json = metapod_to_ascii_json(f);
  ::printf("%s\n", json.c_str());
}

}
