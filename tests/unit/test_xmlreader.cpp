#include <catch2/catch.hpp>
#include "colorer/utils/Environment.h"
#include "colorer/xml/XmlReader.h"
#include "test_common.h"

TEST_CASE("Test read simple xml")
{
  logger->clean_messages();

  UnicodeString path1(u"data/type_cue.hrc");
  XmlInputSource is(path1);
  std::unique_ptr<XmlReader> test_reader;
  REQUIRE_NOTHROW(test_reader = std::make_unique<XmlReader>(is));
  REQUIRE_NOTHROW(test_reader->parse() == true);

  REQUIRE(logger->message_print() == false);
}

TEST_CASE("Test read catalog.xml")
{
  logger->clean_messages();

  UnicodeString path1(u"data/catalog.xml");
  XmlInputSource is(path1);
  std::unique_ptr<XmlReader> test_reader;
  REQUIRE_NOTHROW(test_reader = std::make_unique<XmlReader>(is));
  REQUIRE_NOTHROW(test_reader->parse() == true);

  REQUIRE(logger->message_print() == false);
}

TEST_CASE("Test read entity with env")
{
  logger->clean_messages();

  UnicodeString path1(u"data/catalog-env.xml");
  auto work_dir = fs::current_path();
  colorer::Environment::setOSEnv("CUR_DIR", work_dir.c_str());
  XmlInputSource is(path1);
  std::unique_ptr<XmlReader> test_reader;
  REQUIRE_NOTHROW(test_reader = std::make_unique<XmlReader>(is));
  REQUIRE_NOTHROW(test_reader->parse() == true);

  REQUIRE(logger->message_print() == false);
}

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
TEST_CASE("Test read allpacked catalog.xml")
{
  logger->clean_messages();

  UnicodeString path1(u"data/catalog-allpacked.xml");
  XmlInputSource is(path1);
  std::unique_ptr<XmlReader> test_reader;
  REQUIRE_NOTHROW(test_reader = std::make_unique<XmlReader>(is));
  REQUIRE_NOTHROW(test_reader->parse() == true);

  REQUIRE(logger->message_print() == false);
}

TEST_CASE("Test read jar entity with env")
{
  logger->clean_messages();

  UnicodeString path1(u"data/catalog-allpacked-env.xml");
  auto work_dir = fs::current_path();
  colorer::Environment::setOSEnv("CUR_DIR", work_dir.c_str());
  XmlInputSource is(path1);
  std::unique_ptr<XmlReader> test_reader;
  REQUIRE_NOTHROW(test_reader = std::make_unique<XmlReader>(is));
  REQUIRE_NOTHROW(test_reader->parse() == true);

  REQUIRE(logger->message_print() == false);
}

#endif
