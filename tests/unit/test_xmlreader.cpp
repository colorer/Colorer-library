#include <catch2/catch_amalgamated.hpp>
#include "colorer/utils/Environment.h"
#include "colorer/xml/XmlReader.h"
#include "test_common.h"

TEST_CASE("Test read simple xml", "[xmlreader]")
{
  logger->clean_messages();

  UnicodeString path1(u"data/type_cue.hrc");
  XmlInputSource is(path1);
  std::unique_ptr<XmlReader> test_reader;
  REQUIRE_NOTHROW(test_reader = std::make_unique<XmlReader>(is));
  REQUIRE_NOTHROW(test_reader->parse() == true);

  REQUIRE(logger->message_print() == false);
}

TEST_CASE("Parse captures element text and attributes", "[xmlreader]")
{
  logger->clean_messages();

  UnicodeString path1(u"data/type_cue.hrc");
  XmlInputSource is(path1);
  XmlReader reader(is);
  REQUIRE(reader.parse());

  XMLNodeList nodes;
  reader.getNodes(nodes);
  REQUIRE_FALSE(nodes.empty());
  REQUIRE(nodes.front().name == UnicodeString(u"hrc"));

  const XMLNode* proto = nullptr;
  for (const auto& child : nodes.front().children) {
    if (child.name == UnicodeString(u"prototype")) {
      proto = &child;
      break;
    }
  }
  REQUIRE(proto != nullptr);
  REQUIRE(proto->getAttrValue(UnicodeString(u"name")) == UnicodeString(u"cue"));

  bool found_filename = false;
  for (const auto& child : proto->children) {
    if (child.name == UnicodeString(u"filename")) {
      found_filename = true;
      REQUIRE(child.text.indexOf(UnicodeString(u"cue")) != -1);
    }
  }
  REQUIRE(found_filename);
  REQUIRE(logger->message_print() == false);
}

TEST_CASE("Test read catalog.xml", "[xmlreader]")
{
  logger->clean_messages();

  UnicodeString path1(u"data/catalog.xml");
  XmlInputSource is(path1);
  std::unique_ptr<XmlReader> test_reader;
  REQUIRE_NOTHROW(test_reader = std::make_unique<XmlReader>(is));
  REQUIRE_NOTHROW(test_reader->parse() == true);

  XMLNodeList nodes;
  test_reader->getNodes(nodes);
  REQUIRE_FALSE(nodes.empty());
  REQUIRE(nodes.front().name == UnicodeString(u"catalog"));
  REQUIRE_FALSE(nodes.front().children.empty());

  REQUIRE(logger->message_print() == false);
}

TEST_CASE("Test read entity with env", "[xmlreader]")
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
TEST_CASE("Test read allpacked catalog.xml", "[xmlreader]")
{
  logger->clean_messages();

  UnicodeString path1(u"data/catalog-allpacked.xml");
  XmlInputSource is(path1);
  std::unique_ptr<XmlReader> test_reader;
  REQUIRE_NOTHROW(test_reader = std::make_unique<XmlReader>(is));
  REQUIRE_NOTHROW(test_reader->parse() == true);

  REQUIRE(logger->message_print() == false);
}

TEST_CASE("Test read jar entity with env", "[xmlreader]")
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

TEST_CASE("Sequential XML parses on one thread keep independent entity bases", "[xmlreader]")
{
  logger->clean_messages();
  auto work_dir = fs::current_path();
  colorer::Environment::setOSEnv("CUR_DIR", work_dir.c_str());

  auto parse_catalog = [](const char16_t* path) {
    UnicodeString p(path);
    XmlInputSource is(p);
    XmlReader reader(is);
    REQUIRE(reader.parse());
    XMLNodeList nodes;
    reader.getNodes(nodes);
    REQUIRE_FALSE(nodes.empty());
    REQUIRE(nodes.begin()->name == UnicodeString(u"catalog"));
  };

  parse_catalog(u"data/catalog.xml");
  parse_catalog(u"data/catalog-env.xml");
  parse_catalog(u"data/catalog.xml");
  REQUIRE(logger->message_print() == false);
}
