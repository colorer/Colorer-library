#include <catch2/catch_amalgamated.hpp>
#include "colorer/HrcLibrary.h"
#include "colorer/ParserFactory.h"
#include "colorer/utils/FileSystems.h"
#include "colorer/xml/XmlInputSource.h"

TEST_CASE("A probe ParserFactory can load HRC without disturbing a live instance", "[parserfactory]")
{
  auto hrc_path = fs::path(__FILE__).parent_path() / "data" / "type_tryline.hrc";
  UnicodeString location(hrc_path.c_str());

  ParserFactory live;
  live.loadHrcPath(&location);
  auto* live_type = live.getHrcLibrary().getFileType(UnicodeString("try_line"));
  REQUIRE(live_type != nullptr);
  live.getHrcLibrary().loadFileType(live_type);
  REQUIRE(live_type->getBaseScheme() != nullptr);

  {
    ParserFactory probe;
    probe.loadHrcPath(&location);
    auto* probe_type = probe.getHrcLibrary().getFileType(UnicodeString("try_line"));
    REQUIRE(probe_type != nullptr);
    probe.getHrcLibrary().loadFileType(probe_type);
    REQUIRE(probe_type->getBaseScheme() != nullptr);
    REQUIRE(probe_type != live_type);
  }

  REQUIRE(live.getHrcLibrary().getFileType(UnicodeString("try_line")) == live_type);
  REQUIRE(live_type->getBaseScheme() != nullptr);
}

TEST_CASE("Two ParserFactory instances can parse the same catalog with entities", "[parserfactory]")
{
  auto catalog = fs::path(__FILE__).parent_path() / "data" / "catalog.xml";
  UnicodeString path(catalog.c_str());

  ParserFactory live;
  live.loadCatalog(&path);

  ParserFactory probe;
  probe.loadCatalog(&path);
}

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
TEST_CASE("Two ParserFactory instances can parse the same packed catalog", "[parserfactory]")
{
  auto catalog = fs::path(__FILE__).parent_path() / "data" / "catalog-allpacked.xml";
  UnicodeString path(catalog.c_str());

  ParserFactory live;
  live.loadCatalog(&path);

  ParserFactory probe;
  probe.loadCatalog(&path);
}
#endif
