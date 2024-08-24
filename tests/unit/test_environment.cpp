#include <colorer/Common.h>
#include <colorer/utils/Environment.h>
#include <catch2/catch.hpp>
#include <cstdlib>

TEST_CASE("Test normalize base path")
{
#ifdef WIN32
  UnicodeString path1(R"(c:\testdir\..\windows\.\win.txt)");
  auto norm_path = colorer::Environment::normalizePath(&path1);
  REQUIRE_THAT(R"(c:\windows\win.txt)", Catch::Equals(UStr::to_stdstr(norm_path)));
#else
  UnicodeString path1(R"(/home/user1/testdir/../win/./win.txt)");
  auto norm_path = colorer::Environment::normalizePath(&path1);
  REQUIRE_THAT(R"(/home/user1/win/win.txt)", Catch::Equals(UStr::to_stdstr(norm_path)));
#endif
}

TEST_CASE("Test normalize path with variable")
{
#ifdef WIN32
  _putenv_s("COLORER_TEST_P1", "c:\\testdir");
  UnicodeString path1(R"(%COLORER_TEST_P1%\..\windows\.\win.txt)");
  auto norm_path = colorer::Environment::normalizePath(&path1);
  REQUIRE_THAT(R"(c:\windows\win.txt)", Catch::Equals(UStr::to_stdstr(norm_path)));
#else
  setenv("COLORER_TEST_P1", "/home/user1/testdir", 1);
  UnicodeString path1(R"(${COLORER_TEST_P1}/../win/./win.txt)");
  auto norm_path = colorer::Environment::normalizePath(&path1);
  REQUIRE_THAT(R"(/home/user1/win/win.txt)", Catch::Equals(UStr::to_stdstr(norm_path)));
#endif
}
