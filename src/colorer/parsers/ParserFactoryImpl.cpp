#ifdef __unix__
#include <dirent.h>
#include <sys/stat.h>
#endif
#ifdef WIN32
#include <io.h>
#include <windows.h>
#endif

#include <colorer/common/UStr.h>
#include <colorer/parsers/CatalogParser.h>
#include <colorer/parsers/HRCParserImpl.h>
#include <colorer/parsers/ParserFactoryImpl.h>
#include <colorer/utils/Environment.h>
#include <colorer/viewer/TextLinesStore.h>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/BinFileInputStream.hpp>

ParserFactory::Impl::Impl()
{
  // init xercesc, need to work with xml string
  xercesc::XMLPlatformUtils::Initialize();

  // cregexp.h
  RegExpStack = nullptr;
  RegExpStack_Size = 0;

  hrc_parser = new HRCParser();
}

ParserFactory::Impl::~Impl()
{
  delete hrc_parser;
  delete[] RegExpStack;
  xercesc::XMLPlatformUtils::Terminate();
}

void ParserFactory::Impl::loadCatalog(const UnicodeString* catalog_path)
{
  if (!catalog_path || catalog_path->isEmpty()) {
    auto env = Environment::getOSVariable("COLORER_CATALOG");
    if (!env || env->isEmpty()) {
      throw ParserFactoryException("Can't find suitable catalog.xml for parse.");
    }
    base_catalog_path = UnicodeString(*env);
  } else {
    base_catalog_path = UnicodeString(*catalog_path);
  }

  parseCatalog(base_catalog_path);
  spdlog::debug("start load hrc files");
  for (const auto& location : hrc_locations) {
    try {
      spdlog::debug("try load '{0}'", location);
      auto clear_path = XmlInputSource::getClearPath(&base_catalog_path, &location);
      if (XmlInputSource::isDirectory(clear_path.get())) {
        std::vector<UnicodeString> paths;
        XmlInputSource::getFileFromDir(clear_path.get(), paths);
        for (const auto& files : paths) {
          loadHrc(&files, &base_catalog_path);
        }
      } else {
        loadHrc(clear_path.get(), &base_catalog_path);
      }
    } catch (const Exception& e) {
      spdlog::error("{0}", e.what());
    }
  }

  spdlog::debug("end load hrc files");
}

void ParserFactory::Impl::loadHrc(const UnicodeString* hrc_path, const UnicodeString* base_path) const
{
  uXmlInputSource dfis = XmlInputSource::newInstance(UStr::to_xmlch(hrc_path).get(), UStr::to_xmlch(base_path).get());
  try {
    hrc_parser->loadSource(dfis.get());
  } catch (Exception& e) {
    spdlog::error("Can't load hrc: {0}", UStr::to_stdstr(dfis->getInputSource()->getSystemId()));
    spdlog::error("{0}", e.what());
  }
}

void ParserFactory::Impl::parseCatalog(const UnicodeString& catalog_path)
{
  CatalogParser catalog_parser;
  catalog_parser.parse(&catalog_path);

  hrc_locations.clear();
  hrd_nodes.clear();
  std::copy(catalog_parser.hrc_locations.begin(), catalog_parser.hrc_locations.end(), std::back_inserter(hrc_locations));

  for (auto& item : catalog_parser.hrd_nodes) {
    addHrd(std::move(item));
  }
}

size_t ParserFactory::Impl::countHRD(const UnicodeString& classID)
{
  auto hash = hrd_nodes.find(classID);
  if (hash == hrd_nodes.end()) {
    return 0;
  }
  return hash->second->size();
}

std::vector<UnicodeString> ParserFactory::Impl::enumHRDClasses()
{
  std::vector<UnicodeString> result;
  result.reserve(hrd_nodes.size());
  for (auto& hrd_node : hrd_nodes) {
    result.push_back(hrd_node.first);
  }
  return result;
}

std::vector<const HRDNode*> ParserFactory::Impl::enumHRDInstances(const UnicodeString& classID)
{
  auto hash = hrd_nodes.find(classID);
  std::vector<const HRDNode*> result;
  result.reserve(hash->second->size());
  for (auto& p : *hash->second) {
    result.push_back(p.get());
  }
  return result;
}

const HRDNode* ParserFactory::Impl::getHRDNode(const UnicodeString& classID, const UnicodeString& nameID)
{
  auto hash = hrd_nodes.find(classID);
  if (hash == hrd_nodes.end()) {
    throw ParserFactoryException("can't find HRDClass '" + classID + "'");
  }
  for (auto& p : *hash->second) {
    if (nameID.compare(p->hrd_name) == 0) {
      return p.get();
    }
  }
  throw ParserFactoryException("can't find HRDName '" + nameID + "'");
}

HRCParser* ParserFactory::Impl::getHRCParser() const
{
  return hrc_parser;
}

TextParser* ParserFactory::Impl::createTextParser()
{
  return new TextParser();
}

StyledHRDMapper* ParserFactory::Impl::createStyledMapper(const UnicodeString* classID, const UnicodeString* nameID)
{
  const UnicodeString* class_id;
  const UnicodeString class_default("rgb");
  if (classID == nullptr) {
    class_id = &class_default;
  } else {
    class_id = classID;
  }

  const UnicodeString* name_id;
  const UnicodeString name_default("default");
  UnicodeString name_env;
  if (nameID == nullptr) {
    auto hrd = Environment::getOSVariable("COLORER5HRD");
    if (hrd) {
      name_env = UnicodeString(*hrd);
      name_id = &name_env;
    } else {
      name_id = &name_default;
    }
  } else {
    name_id = nameID;
  }

  auto hrd_node = getHRDNode(*class_id, *name_id);

  auto* mapper = new StyledHRDMapper();
  for (const auto& idx : hrd_node->hrd_location)
    if (idx.length() != 0) {
      uXmlInputSource dfis;
      try {
        dfis = XmlInputSource::newInstance(UStr::to_xmlch(&idx).get(), UStr::to_xmlch(&base_catalog_path).get());
        mapper->loadRegionMappings(dfis.get());
      } catch (Exception& e) {
        spdlog::error("Can't load hrd:");
        spdlog::error("{0}", e.what());
        throw ParserFactoryException("Error load hrd");
      }
    }
  return mapper;
}

TextHRDMapper* ParserFactory::Impl::createTextMapper(const UnicodeString* nameID)
{
  // fixed class 'text'
  UnicodeString d_text = UnicodeString("text");

  const UnicodeString* name_id;
  const UnicodeString name_default("default");
  if (nameID == nullptr) {
    name_id = &name_default;
  } else {
    name_id = nameID;
  }

  auto hrd_node = getHRDNode(d_text, *name_id);

  auto* mapper = new TextHRDMapper();
  for (const auto& idx : hrd_node->hrd_location)
    if (idx.length() != 0) {
      uXmlInputSource dfis;
      try {
        dfis = XmlInputSource::newInstance(UStr::to_xmlch(&idx).get(), UStr::to_xmlch(&base_catalog_path).get());
        mapper->loadRegionMappings(dfis.get());
      } catch (Exception& e) {
        spdlog::error("Can't load hrd: ");
        spdlog::error("{0}", e.what());
      }
    }
  return mapper;
}

void ParserFactory::Impl::addHrd(std::unique_ptr<HRDNode> hrd)
{
  if (hrd_nodes.find(hrd->hrd_class) == hrd_nodes.end()) {
    hrd_nodes.emplace(hrd->hrd_class, std::make_unique<std::vector<std::unique_ptr<HRDNode>>>());
  }
  hrd_nodes.at(hrd->hrd_class)->emplace_back(std::move(hrd));
}
