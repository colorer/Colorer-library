#include <catch2/catch_amalgamated.hpp>
#include <system_error>
#include "colorer/FileType.h"
#include "colorer/HrcLibrary.h"
#include "colorer/Scheme.h"
#include "colorer/utils/Environment.h"
#include "colorer/utils/FileSystems.h"
#include "colorer/xml/XmlInputSource.h"

namespace {

XmlInputSource hrcInput(const char* file_name)
{
  auto path = fs::path(__FILE__).parent_path() / "data" / file_name;
  return XmlInputSource(UnicodeString(path.c_str()), nullptr);
}

}  // namespace

TEST_CASE("Load hrc", "[hrc]")
{
  XmlInputSource file1 = hrcInput("type_cue.hrc");
  HrcLibrary lib;
  lib.loadSource(&file1);
}

TEST_CASE("Load HRC expands a SYSTEM entity into the type tree", "[hrc]")
{
  XmlInputSource file = hrcInput("type_entity_incl.hrc");
  HrcLibrary lib;
  lib.loadSource(&file);
  auto* type = lib.getFileType(UnicodeString("entinc"));
  REQUIRE(type != nullptr);
  REQUIRE(type->getBaseScheme() != nullptr);
  REQUIRE(*type->getBaseScheme()->getName() == UnicodeString("entinc:entinc"));
}

TEST_CASE("Duplicate prototype replaces the FileType shell before type load", "[hrc]")
{
  XmlInputSource base = hrcInput("ov_base.hrc");
  XmlInputSource repl = hrcInput("ov_replace.hrc");
  HrcLibrary lib;
  lib.loadProtoTypes(&base);
  lib.loadProtoTypes(&repl);

  auto* type = lib.getFileType(UnicodeString("ov"));
  REQUIRE(type != nullptr);
  REQUIRE(type->getDescription() == UnicodeString("overlay replace"));
  REQUIRE(*type->getParamValue(UnicodeString("flag")) == UnicodeString("replace"));

  UnicodeString repl_name(".repl");
  REQUIRE(lib.chooseFileType(&repl_name, nullptr) == type);

  lib.loadFileType(type);
  REQUIRE(type->getBaseScheme() != nullptr);
}

TEST_CASE("hrcsettings updates a prototype before type load", "[hrc]")
{
  XmlInputSource base = hrcInput("ov_base.hrc");
  XmlInputSource settings = hrcInput("ov_settings.xml");
  HrcLibrary lib;
  lib.loadProtoTypes(&base);
  lib.loadHrcSettings(settings);

  auto* type = lib.getFileType(UnicodeString("ov"));
  REQUIRE(type != nullptr);
  REQUIRE(*type->getParamValue(UnicodeString("flag")) == UnicodeString("settings"));

  UnicodeString set_name(".set");
  REQUIRE(lib.chooseFileType(&set_name, nullptr) == type);
}

TEST_CASE("Duplicate prototype after type load is an error", "[hrc]")
{
  XmlInputSource base = hrcInput("ov_base.hrc");
  XmlInputSource repl = hrcInput("ov_replace.hrc");
  HrcLibrary lib;
  lib.loadProtoTypes(&base);
  auto* type = lib.getFileType(UnicodeString("ov"));
  lib.loadFileType(type);

  REQUIRE_THROWS_AS(lib.loadProtoTypes(&repl), HrcLibraryException);
  REQUIRE_THROWS_WITH(lib.loadProtoTypes(&repl),
                      Catch::Matchers::ContainsSubstring("before type content is loaded"));

  auto* still = lib.getFileType(UnicodeString("ov"));
  REQUIRE(still == type);
  REQUIRE(still->getDescription() == UnicodeString("overlay base"));
}

TEST_CASE("hrcsettings after type load is an error", "[hrc]")
{
  XmlInputSource base = hrcInput("ov_base.hrc");
  XmlInputSource settings = hrcInput("ov_settings.xml");
  HrcLibrary lib;
  lib.loadProtoTypes(&base);
  lib.loadFileType(lib.getFileType(UnicodeString("ov")));

  REQUIRE_THROWS_AS(lib.loadHrcSettings(settings), HrcLibraryException);
  REQUIRE_THROWS_WITH(lib.loadHrcSettings(settings),
                      Catch::Matchers::ContainsSubstring("before type content is loaded"));
  REQUIRE(*lib.getFileType(UnicodeString("ov"))->getParamValue(UnicodeString("flag")) == UnicodeString("base"));
}

TEST_CASE("Prototype overlay of a new name is refused after another type is loaded", "[hrc]")
{
  XmlInputSource base = hrcInput("ov_base.hrc");
  XmlInputSource cue = hrcInput("type_cue.hrc");
  HrcLibrary lib;
  lib.loadProtoTypes(&base);
  lib.loadFileType(lib.getFileType(UnicodeString("ov")));

  REQUIRE_THROWS_AS(lib.loadProtoTypes(&cue), HrcLibraryException);
  REQUIRE(lib.getFileType(UnicodeString("cue")) == nullptr);
}

TEST_CASE("Full load then duplicate prototype is an error", "[hrc]")
{
  XmlInputSource base = hrcInput("ov_base.hrc");
  XmlInputSource repl = hrcInput("ov_replace.hrc");
  HrcLibrary lib;
  lib.loadSource(&base);

  REQUIRE_THROWS_AS(lib.loadSource(&repl), HrcLibraryException);
  REQUIRE_THROWS_WITH(lib.loadSource(&repl),
                      Catch::Matchers::ContainsSubstring("type content is already loaded"));

  auto* type = lib.getFileType(UnicodeString("ov"));
  REQUIRE(type != nullptr);
  REQUIRE(type->getDescription() == UnicodeString("overlay base"));
  REQUIRE(type->getBaseScheme() != nullptr);
}

TEST_CASE("HRC SYSTEM entity loads from a path with spaces and Cyrillic", "[hrc]")
{
  const auto root = fs::temp_directory_path() / fs::u8path("colorer_hrc_русс еще с");
  fs::remove_all(root);
  fs::create_directories(root);
  struct RemoveAll {
    fs::path p;
    ~RemoveAll()
    {
      std::error_code ec;
      fs::remove_all(p, ec);
    }
  } cleanup {root};

  const auto data = fs::path(__FILE__).parent_path() / "data";
  fs::copy_file(data / "type_entity_incl.hrc", root / "type_entity_incl.hrc");
  fs::copy_file(data / "type_entity_frag.hrc", root / "type_entity_frag.hrc");

  XmlInputSource file(colorer::Environment::from_filepath(root / "type_entity_incl.hrc"), nullptr);
  HrcLibrary lib;
  REQUIRE_NOTHROW(lib.loadSource(&file));
  auto* type = lib.getFileType(UnicodeString("entinc"));
  REQUIRE(type != nullptr);
  REQUIRE(type->getBaseScheme() != nullptr);
  REQUIRE(*type->getBaseScheme()->getName() == UnicodeString("entinc:entinc"));
}
