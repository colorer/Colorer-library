#include <cstdio>
#include <cstdlib>
#ifdef __unix__
#include <dirent.h>
#include <sys/stat.h>
#endif
#ifdef WIN32
#include <io.h>
#include <windows.h>
#endif

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/BinFileInputStream.hpp>

#include <colorer/parsers/ParserFactory.h>
#include <colorer/parsers/CatalogParser.h>
#include <colorer/viewer/TextLinesStore.h>
#include <colorer/parsers/HRCParserImpl.h>
#include <colorer/parsers/TextParserImpl.h>
#include <colorer/parsers/ParserFactoryException.h>

#include <colorer/xml/XmlInputSource.h>
#include <colorer/xml/XStr.h>
#include "ParserFactory.h"
#include <colorer/common/UnicodeLogger.h>


ParserFactory::ParserFactory(): hrc_parser(new HRCParserImpl())
{
  RegExpStack = nullptr;
  RegExpStack_Size = 0;
}

ParserFactory::~ParserFactory()
{
  delete hrc_parser;
  delete[] RegExpStack;
}

UnicodeString ParserFactory::searchCatalog() const
{
  spdlog::debug("begin search catalog.xml");

  std::vector<UnicodeString> paths;
  getPossibleCatalogPaths(paths);

  UnicodeString right_path;
  for (const auto& path : paths) {
    try {
      spdlog::debug("test path '{0}'", path);

      uXmlInputSource catalog = XmlInputSource::newInstance(UStr::to_xmlch(&path).get(), static_cast<XMLCh*>(nullptr));

      std::unique_ptr<xercesc::BinInputStream> stream(catalog->makeStream());
      right_path = UnicodeString(catalog->getInputSource()->getSystemId());

      spdlog::debug("found valid path '{0}' = '{1}'", path, right_path);
      break;
    } catch (const Exception &e) {
      spdlog::error( e.what());
    }
  }
  spdlog::debug("end search catalog.xml");
  if (right_path.length() == 0) {
    spdlog::error("Can't find suitable catalog.xml file. Check your program settings.");
    throw ParserFactoryException("Can't find suitable catalog.xml file. Check your program settings.");
  }
  return right_path;
}

#ifdef WIN32
void ParserFactory::getPossibleCatalogPaths(std::vector<UnicodeString> &paths) const
{
  // image_path/  image_path/..
  HMODULE hmod = GetModuleHandle(nullptr);
  if (hmod) {
    wchar_t cname[MAX_PATH];
    int len = GetModuleFileNameW(hmod, cname, MAX_PATH);
    if (len > 0) {
      UnicodeString module(cname, 0, len - 1);
      int pos[2];
      pos[0] = module.lastIndexOf('\\');
      pos[1] = module.lastIndexOf('\\', pos[0]);
      for (int idx = 0; idx < 2; idx++)
        if (pos[idx] >= 0) {
          paths.emplace_back(UnicodeString(UnicodeString(module, 0, pos[idx])).append("\\catalog.xml"));
        }
    }
  }

  // %COLORER5CATALOG%
  char* colorer5_catalog = getenv("COLORER5CATALOG");
  if (colorer5_catalog) {
    paths.emplace_back(UnicodeString(colorer5_catalog));
  }

  // %HOMEDRIVE%%HOMEPATH%\.colorer5catalog
  char* home_drive = getenv("HOMEDRIVE");
  char* home_path = getenv("HOMEPATH");
  if (home_drive && home_path) {
    try {
      UnicodeString d = UnicodeString(home_drive).append(UnicodeString(home_path)).append(UnicodeString("/.colorer5catalog"));
      if (_access(UStr::to_stdstr(&d).c_str(), 0) != -1) {
        TextLinesStore tls;
        tls.loadFile(&d, nullptr, false);
        if (tls.getLineCount() > 0) {
          paths.emplace_back(UnicodeString(*tls.getLine(0)));
        }
      }
    } catch (InputSourceException &) { //-V565
      // it`s ok. the error is not interesting
    }
  }
}
#endif

#ifdef __unix__
void ParserFactory::getPossibleCatalogPaths(std::vector<UnicodeString> &paths) const
{
  // %COLORER5CATALOG%
  char* colorer5_catalog = getenv("COLORER5CATALOG");
  if (colorer5_catalog) {
    paths.emplace_back(UnicodeString(colorer5_catalog));
  }

  // %HOME%/.colorer5catalog
  char* home_path = getenv("HOME");
  if (home_path != nullptr) {
    try {
      TextLinesStore tls;
      tls.loadFile(&UnicodeString(home_path).append("/.colorer5catalog"), nullptr, false);
      if (tls.getLineCount() > 0) {
        paths.emplace_back(*tls.getLine(0));
      }
    } catch (InputSourceException &) { //-V565
      // it`s ok. the error is not interesting
    }
  }

  // /usr/share/colorer/catalog.xml
  paths.emplace_back(UnicodeString("/usr/share/colorer/catalog.xml"));
  paths.emplace_back(UnicodeString("/usr/local/share/colorer/catalog.xml"));
}
#endif

void ParserFactory::loadCatalog(const UnicodeString* catalog_path)
{
  if (!catalog_path) {
    base_catalog_path = searchCatalog();
    if (base_catalog_path.length() == 0) {
      throw ParserFactoryException("Can't find suitable catalog.xml file.");
    }
  } else {
    base_catalog_path = UnicodeString(*catalog_path);
  }


  parseCatalog(base_catalog_path);
  spdlog::debug("begin load hrc files");
  for (auto location : hrc_locations) {
    try {
      spdlog::debug("try load '{0}'", location);
      auto clear_path = XmlInputSource::getClearPath(&base_catalog_path, &location);
      if (XmlInputSource::isDirectory(clear_path.get())) {
        std::vector<UnicodeString> paths;
        XmlInputSource::getFileFromDir(clear_path.get(), paths);
        for (auto files : paths) {
          loadHrc(&files, &base_catalog_path);
        }
      } else {
        loadHrc(clear_path.get(), &base_catalog_path);
      }
    } catch (const Exception &e) {
      spdlog::error("{0}", e.what());
    }
  }

  spdlog::debug("end load hrc files");
}

void ParserFactory::loadHrc(const UnicodeString* hrc_path, const UnicodeString* base_path) const
{
  uXmlInputSource dfis = XmlInputSource::newInstance(UStr::to_xmlch(hrc_path).get(), UStr::to_xmlch(base_path).get());
  try {
    hrc_parser->loadSource(dfis.get());
  } catch (Exception &e) {
    spdlog::error("Can't load hrc: {0}", XStr(dfis->getInputSource()->getSystemId()).get_char());
    spdlog::error("{0}", e.what());
  }
}

void ParserFactory::parseCatalog(const UnicodeString &catalog_path)
{
  hrc_locations.clear();
  hrd_nodes.clear();

  CatalogParser catalog_parser;
  catalog_parser.parse(&catalog_path);

  for (const auto& hrc_location : catalog_parser.hrc_locations) {
    hrc_locations.push_back(hrc_location);
  }

  while (!catalog_parser.hrd_nodes.empty()) {
    auto hrd = std::move(catalog_parser.hrd_nodes.front());
    catalog_parser.hrd_nodes.pop_front();
    addHrd(std::move(hrd));
  }
}

const char* ParserFactory::getVersion()
{
  return "Colorer-take5 Library be5 28 May 2006";
}

size_t ParserFactory::countHRD(const UnicodeString &classID)
{
  auto hash = hrd_nodes.find(classID);
  if (hash == hrd_nodes.end()) {
    return 0;
  }
  return hash->second->size();
}

std::vector<UnicodeString> ParserFactory::enumHRDClasses()
{
  std::vector<UnicodeString> result;
  result.reserve(hrd_nodes.size());
  for (auto & hrd_node : hrd_nodes) {
    result.push_back(hrd_node.first);
  }
  return result;
}

std::vector<const HRDNode*> ParserFactory::enumHRDInstances(const UnicodeString &classID)
{
  auto hash = hrd_nodes.find(classID);
  std::vector<const HRDNode*> result;
  result.reserve(hash->second->size());
  for (auto & p : *hash->second) {
    result.push_back(p.get());
  }
  return result;
}

const HRDNode* ParserFactory::getHRDNode(const UnicodeString &classID, const UnicodeString &nameID)
{
  auto hash = hrd_nodes.find(classID);
  if (hash == hrd_nodes.end()) {
    throw ParserFactoryException("can't find HRDClass '" + classID + "'");
  }
  for (auto & p : *hash->second) {
    if (nameID.compare(p->hrd_name) == 0) {
      return p.get();
    }
  }
  throw ParserFactoryException("can't find HRDName '" + nameID + "'");
}

HRCParser* ParserFactory::getHRCParser() const
{
  return hrc_parser;
}

TextParser* ParserFactory::createTextParser()
{
  return new TextParserImpl();
}

StyledHRDMapper* ParserFactory::createStyledMapper(const UnicodeString* classID, const UnicodeString* nameID)
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
    char* hrd = getenv("COLORER5HRD");
    if (hrd != nullptr) {
      name_env = UnicodeString(hrd);
      name_id = &name_env;
    } else {
      name_id = &name_default;
    }
  } else {
    name_id = nameID;
  }

  auto hrd_node = getHRDNode(*class_id, *name_id);

  auto* mapper = new StyledHRDMapper();
  for (const auto & idx : hrd_node->hrd_location)
    if (idx.length() != 0) {
      uXmlInputSource dfis = nullptr;
      try {
        dfis = XmlInputSource::newInstance(UStr::to_xmlch(&idx).get(), UStr::to_xmlch(&base_catalog_path).get());
        mapper->loadRegionMappings(dfis.get());
      } catch (Exception &e) {
        spdlog::error("Can't load hrd:");
        spdlog::error("{0}", e.what());
        throw ParserFactoryException("Error load hrd");
      }
    }
  return mapper;
}

TextHRDMapper* ParserFactory::createTextMapper(const UnicodeString* nameID)
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
  for (const auto & idx : hrd_node->hrd_location)
    if (idx.length() != 0) {
      uXmlInputSource dfis = nullptr;
      try {
        dfis = XmlInputSource::newInstance(UStr::to_xmlch(&idx).get(), UStr::to_xmlch(&base_catalog_path).get());
        mapper->loadRegionMappings(dfis.get());
      } catch (Exception &e) {
        spdlog::error("Can't load hrd: ");
        spdlog::error("{0}", e.what());
      }
    }
  return mapper;
}

void ParserFactory::addHrd(std::unique_ptr<HRDNode> hrd)
{
  if (hrd_nodes.find(hrd->hrd_class) == hrd_nodes.end()) {
    hrd_nodes.emplace(hrd->hrd_class, std::make_unique<std::vector<std::unique_ptr<HRDNode>>>());
  }
  hrd_nodes.at(hrd->hrd_class)->emplace_back(std::move(hrd));
}



