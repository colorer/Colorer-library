#include <colorer/Common.h>
#include <colorer/xml/xercesc/XercesXmlInputSource.h>
#include <catch2/catch.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

TEST_CASE("Test create instance")
{
  xercesc::XMLPlatformUtils::Initialize();
  SECTION("creat instances with not exists file")
  {
    const auto empty_string = (const XMLCh *) u"";
    const auto path_string = (const XMLCh *) u"c:\\windows";

    REQUIRE_THROWS_WITH(XercesXmlInputSource::newInstance((UnicodeString*) nullptr, nullptr), Catch::Contains("path is empty"));
    REQUIRE_THROWS_WITH(XercesXmlInputSource::newInstance(empty_string, nullptr), Catch::Contains("path is empty"));
    REQUIRE_THROWS_WITH(XercesXmlInputSource::newInstance(empty_string, empty_string), Catch::Contains("path is empty"));
    REQUIRE_THROWS_WITH(XercesXmlInputSource::newInstance(empty_string, path_string), Catch::Contains("path is empty"));

    auto s1 = UnicodeString(u"../");
    auto s2 = UnicodeString(u"c:\\windows");
    REQUIRE_THROWS_WITH(XercesXmlInputSource::newInstance(&s1, nullptr), Catch::Contains("isn't regular file"));
    REQUIRE_THROWS_WITH(XercesXmlInputSource::newInstance(&s1, &s2), Catch::Contains("isn't regular file"));
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

    uXercesXmlInputSource a;
    auto s1 = UnicodeString(test_file.c_str());
    REQUIRE_NOTHROW(a = XercesXmlInputSource::newInstance(&s1, nullptr));
    REQUIRE_NOTHROW(a->makeStream());
  }
  std::error_code ec;
  std::filesystem::remove_all(work_dir, ec);
  xercesc::XMLPlatformUtils::Terminate();
}