#include <catch2/catch_amalgamated.hpp>
#include <fstream>
#include <system_error>
#include "colorer/ParserFactory.h"
#include "colorer/utils/Environment.h"
#include "colorer/xml/XmlReader.h"
#include "test_common.h"

namespace {

fs::path xmlreaderDataDir()
{
  return fs::path(__FILE__).parent_path() / "data";
}

struct TempTree {
  fs::path root;
  explicit TempTree(fs::path p) : root(std::move(p))
  {
    fs::remove_all(root);
    fs::create_directories(root);
  }
  ~TempTree()
  {
    std::error_code ec;
    fs::remove_all(root, ec);
  }
  TempTree(const TempTree&) = delete;
  TempTree& operator=(const TempTree&) = delete;
};

fs::path copyCatalogWithEntities(const fs::path& dest_root)
{
  fs::create_directories(dest_root / "hrd");
  fs::copy_file(xmlreaderDataDir() / "catalog.xml", dest_root / "catalog.xml");
  fs::copy_file(xmlreaderDataDir() / "hrd" / "catalog-console.xml", dest_root / "hrd" / "catalog-console.xml");
  return dest_root / "catalog.xml";
}

const XMLNode* childNamed(const XMLNode& parent, const char16_t* name)
{
  const UnicodeString wanted(name);
  for (const auto& child : parent.children) {
    if (child.name == wanted) {
      return &child;
    }
  }
  return nullptr;
}

}  // namespace

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

TEST_CASE("Catalog SYSTEM entities load from a path with spaces and Cyrillic", "[xmlreader]")
{
  logger->clean_messages();

  TempTree tree(fs::temp_directory_path() / fs::u8path("colorer_русс еще с"));
  const auto catalog_file = copyCatalogWithEntities(tree.root / "base");
  const auto catalog_path = colorer::Environment::from_filepath(catalog_file);

  XmlInputSource is(catalog_path);
  XmlReader reader(is);
  REQUIRE(reader.parse());

  XMLNodeList nodes;
  reader.getNodes(nodes);
  REQUIRE_FALSE(nodes.empty());
  REQUIRE(nodes.front().name == UnicodeString(u"catalog"));
  const XMLNode* hrd_sets = childNamed(nodes.front(), u"hrd-sets");
  REQUIRE(hrd_sets != nullptr);
  REQUIRE_FALSE(hrd_sets->children.empty());
  REQUIRE(hrd_sets->children.front().name == UnicodeString(u"hrd"));
  REQUIRE(hrd_sets->children.front().getAttrValue(UnicodeString(u"name")) == UnicodeString(u"default"));
  REQUIRE(logger->message_print() == false);

  ParserFactory factory;
  REQUIRE_NOTHROW(factory.loadCatalog(&catalog_path));
  REQUIRE_FALSE(factory.enumHrdInstances(UnicodeString(u"console")).empty());
}

TEST_CASE("Catalog SYSTEM entities ignore a same-name file in the process cwd", "[xmlreader]")
{
  logger->clean_messages();

  TempTree tree(fs::temp_directory_path() / fs::u8path("colorer_ascii path"));
  const auto catalog_file = copyCatalogWithEntities(tree.root / "base");
  const auto catalog_path = colorer::Environment::from_filepath(catalog_file);

  const auto decoy_dir = fs::current_path() / "hrd";
  const auto decoy_file = decoy_dir / "catalog-console.xml";
  const bool created_dir = !fs::exists(decoy_dir);
  REQUIRE_FALSE(fs::exists(decoy_file));
  fs::create_directories(decoy_dir);
  {
    std::ofstream decoy(decoy_file);
    decoy << "<not-hrd/>\n";
  }

  XMLNodeList nodes;
  try {
    XmlInputSource is(catalog_path);
    XmlReader reader(is);
    REQUIRE(reader.parse());
    reader.getNodes(nodes);
  } catch (...) {
    std::error_code ec;
    fs::remove(decoy_file, ec);
    if (created_dir) {
      fs::remove(decoy_dir, ec);
    }
    throw;
  }
  std::error_code ec;
  fs::remove(decoy_file, ec);
  if (created_dir) {
    fs::remove(decoy_dir, ec);
  }

  REQUIRE_FALSE(nodes.empty());
  const XMLNode* hrd_sets = childNamed(nodes.front(), u"hrd-sets");
  REQUIRE(hrd_sets != nullptr);
  REQUIRE_FALSE(hrd_sets->children.empty());
  REQUIRE(hrd_sets->children.front().name == UnicodeString(u"hrd"));
  REQUIRE(logger->message_print() == false);
}
