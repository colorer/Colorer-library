#include <colorer/Exception.h>
#include <spdlog/sinks/null_sink.h>
#include <catch2/catch.hpp>

using Catch::Matchers::Equals;

TEST_CASE("Work with Exception class")
{
  // disable logging
  spdlog::drop_all();
  auto log = spdlog::null_logger_mt("main");
  spdlog::set_default_logger(log);
  
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
