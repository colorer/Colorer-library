#include <colorer/Common.h>
#include <colorer/utils/Environment.h>
#include <catch2/catch_amalgamated.hpp>
#include <cstdlib>
#include <fstream>
#include <system_error>

TEST_CASE("Test normalize base path", "[environment]")
{
#ifdef WIN32
  UnicodeString path1(R"(c:\testdir\..\windows\.\win.txt)");
  auto norm_path = colorer::Environment::normalizePath(&path1);
  REQUIRE_THAT(R"(c:\windows\win.txt)", Catch::Matchers::Equals(UStr::to_stdstr(norm_path)));
#else
  UnicodeString path1(R"(/home/user1/testdir/../win/./win.txt)");
  auto norm_path = colorer::Environment::normalizePath(&path1);
  REQUIRE_THAT(R"(/home/user1/win/win.txt)", Catch::Matchers::Equals(UStr::to_stdstr(norm_path)));
#endif
}

TEST_CASE("Test normalize path with variable", "[environment]")
{
#ifdef WIN32
  _putenv_s("COLORER_TEST_P1", "c:\\testdir");
  UnicodeString path1(R"(%COLORER_TEST_P1%\..\windows\.\win.txt)");
  auto norm_path = colorer::Environment::normalizePath(&path1);
  REQUIRE_THAT(R"(c:\windows\win.txt)", Catch::Matchers::Equals(UStr::to_stdstr(norm_path)));
#else
  setenv("COLORER_TEST_P1", "/home/user1/testdir", 1);
  UnicodeString path1(R"(${COLORER_TEST_P1}/../win/./win.txt)");
  auto norm_path = colorer::Environment::normalizePath(&path1);
  REQUIRE_THAT(R"(/home/user1/win/win.txt)", Catch::Matchers::Equals(UStr::to_stdstr(norm_path)));
#endif
}

TEST_CASE("expandEnvironment leaves paths without $ unchanged", "[environment]")
{
  const UnicodeString path("/usr/share/colorer/catalog.xml");
  REQUIRE(colorer::Environment::expandEnvironment(path) == path);
  REQUIRE(colorer::Environment::expandSpecialEnvironment(path) == path);
}

#ifndef WIN32
TEST_CASE("expandEnvironment substitutes $VAR and ${VAR}", "[environment]")
{
  setenv("COLORER_TEST_P2", "/opt/colorer", 1);

  const UnicodeString brace("${COLORER_TEST_P2}/hrc");
  const auto expanded_brace = colorer::Environment::expandEnvironment(brace);
  REQUIRE_THAT("/opt/colorer/hrc", Catch::Matchers::Equals(UStr::to_stdstr(&expanded_brace)));

  const UnicodeString dollar("$COLORER_TEST_P2/hrc");
  const auto expanded_dollar = colorer::Environment::expandEnvironment(dollar);
  REQUIRE_THAT("/opt/colorer/hrc", Catch::Matchers::Equals(UStr::to_stdstr(&expanded_dollar)));
  const auto special_dollar = colorer::Environment::expandSpecialEnvironment(dollar);
  REQUIRE_THAT("/opt/colorer/hrc", Catch::Matchers::Equals(UStr::to_stdstr(&special_dollar)));
}
#endif

TEST_CASE("normalizePath and isRegularFile keep Cyrillic and spaces", "[environment]")
{
  const auto root = fs::temp_directory_path() / fs::u8path("colorer_env_русс еще с");
  fs::create_directories(root);
  struct RemoveAll {
    fs::path p;
    ~RemoveAll()
    {
      std::error_code ec;
      fs::remove_all(p, ec);
    }
  } cleanup {root};

  const auto file = root / "catalog.xml";
  {
    std::ofstream out(file);
    out << "<catalog/>\n";
  }

  const auto as_unicode = colorer::Environment::from_filepath(file);
  REQUIRE(colorer::Environment::isRegularFile(as_unicode));
  const auto normalized = colorer::Environment::normalizePath(&as_unicode);
  REQUIRE(colorer::Environment::isRegularFile(*normalized));
  REQUIRE(colorer::Environment::to_filepath(normalized.get()) == fs::path(file).lexically_normal());
}

