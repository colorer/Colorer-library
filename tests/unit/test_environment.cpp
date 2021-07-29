#include <colorer/Common.h>
#include <colorer/common/UStr.h>
#include <colorer/utils/Environment.h>
#include <catch2/catch.hpp>

TEST_CASE("Test normalize base path")
{
  UnicodeString path1(R"(c:\testdir\..\windows\.\win.txt)");
  auto norm_path = Environment::normalizePath(&path1);
  REQUIRE_THAT(R"(c:\windows\win.txt)", Catch::Equals(UStr::to_stdstr(norm_path)));
}

TEST_CASE("Test normalize path with variable")
{
  _putenv_s("COLORER_TEST_P1", "c:\\testdir");
  UnicodeString path1(R"(%COLORER_TEST_P1%\..\windows\.\win.txt)");
  auto norm_path = Environment::normalizePath(&path1);
  REQUIRE_THAT(R"(c:\windows\win.txt)", Catch::Equals(UStr::to_stdstr(norm_path)));
}
