#include <colorer/Exception.h>
#include <catch2/catch.hpp>

using Catch::Matchers::Equals;

TEST_CASE("Work with Exception class")
{

  SECTION("check error message if it char string")
  {
    Exception excp("error");
    std::string what = excp.what();
    REQUIRE_THAT("error", Equals(what));
  }
  SECTION("check error message if it UnicodeString")
  {
    Exception excp(UnicodeString("error"));
    std::string what = excp.what();
    REQUIRE_THAT("error", Equals(what));
  }
}
