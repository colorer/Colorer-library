#include <colorer/Common.h>
#include <colorer/xml/XmlInputSource.h>
#include <catch2/catch.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

TEST_CASE("Test create instance")
{
  xercesc::XMLPlatformUtils::Initialize();
  SECTION("creat instances with not exists file")
  {
    REQUIRE_THROWS_WITH(XmlInputSource::newInstance((UnicodeString*) nullptr, nullptr), Catch::Contains("path is empty"));
    REQUIRE_THROWS_WITH(XmlInputSource::newInstance(u"", nullptr), Catch::Contains("path is empty"));
    REQUIRE_THROWS_WITH(XmlInputSource::newInstance(u"", u""), Catch::Contains("path is empty"));
    REQUIRE_THROWS_WITH(XmlInputSource::newInstance(u"", u"c:\\windows"), Catch::Contains("path is empty"));

    auto s1 = UnicodeString(u"../");
    auto s2 = UnicodeString(u"c:\\windows");
    REQUIRE_THROWS_WITH(XmlInputSource::newInstance(&s1, nullptr), Catch::Contains("isn't regular file"));
    REQUIRE_THROWS_WITH(XmlInputSource::newInstance(&s1, &s2), Catch::Contains("isn't regular file"));
  }
  xercesc::XMLPlatformUtils::Terminate();
}

TEST_CASE("Test open instance")
{
  xercesc::XMLPlatformUtils::Initialize();
  auto temp_path = fs::current_path();
  auto work_dir = temp_path / "colorer_test";
  fs::create_directories(work_dir);
  SECTION("open exists file")
  {
    auto test_dir = work_dir / "test1";
    fs::create_directories(test_dir);
    auto test_file = test_dir / "test1.xml";
    std::ofstream(test_file.c_str()).close();

    uXmlInputSource a;
    auto s1 = UnicodeString(test_file.c_str());
    REQUIRE_NOTHROW(a = XmlInputSource::newInstance(&s1, nullptr));
    REQUIRE_NOTHROW(a->makeStream());
  }
  std::error_code ec;
  std::filesystem::remove_all(work_dir, ec);
  xercesc::XMLPlatformUtils::Terminate();
}