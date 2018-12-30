#include <stdio.h>
#include <stdlib.h>
#ifdef __unix__
#include <dirent.h>
#include <sys/stat.h>
#endif
#ifdef _WIN32
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

SString ParserFactory::searchCatalog() const
{
  LOG(G3LOG_DEBUG) << "begin search catalog.xml";

  std::vector<SString> paths;
  getPossibleCatalogPaths(paths);

  SString right_path;
  for (auto path : paths) {
    try {
      LOG(G3LOG_DEBUG) << "test path '" << path.getChars() << "'";

      uXmlInputSource catalog = XmlInputSource::newInstance(path.getWChars(), static_cast<XMLCh*>(nullptr));

      std::unique_ptr<xercesc::BinInputStream> stream(catalog->makeStream());
      right_path = SString(catalog->getInputSource()->getSystemId());

      LOG(G3LOG_DEBUG) << "found valid path '" << path.getChars() << "' = '" << right_path.getChars() << "'";
      break;
    } catch (const Exception &e) {
      LOG(ERROR) << e.what();
    }
  }
  LOG(G3LOG_DEBUG) << "end search catalog.xml";
  if (right_path.length() == 0) {
    LOGF(ERROR, "Can't find suitable catalog.xml file. Check your program settings.");
    throw ParserFactoryException(CString("Can't find suitable catalog.xml file. Check your program settings."));
  }
  return right_path;
}

#ifdef _WIN32
void ParserFactory::getPossibleCatalogPaths(std::vector<SString> &paths) const
{
  // image_path/  image_path/..
  HMODULE hmod = GetModuleHandle(nullptr);
  if (hmod) {
    wchar_t cname[MAX_PATH];
    int len = GetModuleFileName(hmod, cname, MAX_PATH);
    if (len > 0) {
      CString module(cname, 0, len - 1);
      int pos[2];
      pos[0] = module.lastIndexOf('\\');
      pos[1] = module.lastIndexOf('\\', pos[0]);
      for (int idx = 0; idx < 2; idx++)
        if (pos[idx] >= 0) {
          paths.emplace_back(SString(CString(module, 0, pos[idx])).append(CString("\\catalog.xml")));
        }
    }
  }

  // %COLORER5CATALOG%
  char* colorer5_catalog = getenv("COLORER5CATALOG");
  if (colorer5_catalog) {
    paths.emplace_back(SString(colorer5_catalog));
  }

  // %HOMEDRIVE%%HOMEPATH%\.colorer5catalog
  char* home_drive = getenv("HOMEDRIVE");
  char* home_path = getenv("HOMEPATH");
  if (home_drive && home_path) {
    try {
      SString d = SString(home_drive).append(CString(home_path)).append(CString("/.colorer5catalog"));
      if (_access(d.getChars(), 0) != -1) {
        TextLinesStore tls;
        tls.loadFile(&d, nullptr, false);
        if (tls.getLineCount() > 0) {
          paths.emplace_back(SString(tls.getLine(0)));
        }
      }
    } catch (InputSourceException &) { //-V565
      // it`s ok. the error is not interesting
    }
  }
}
#endif

#ifdef __unix__
void ParserFactory::getPossibleCatalogPaths(std::vector<SString> &paths) const
{
  // %COLORER5CATALOG%
  char* colorer5_catalog = getenv("COLORER5CATALOG");
  if (colorer5_catalog) {
    paths.emplace_back(SString(colorer5_catalog));
  }

  // %HOME%/.colorer5catalog
  char* home_path = getenv("HOME");
  if (home_path != nullptr) {
    try {
      TextLinesStore tls;
      tls.loadFile(&SString(home_path).append(CString("/.colorer5catalog")), nullptr, false);
      if (tls.getLineCount() > 0) {
        paths.emplace_back(SString(tls.getLine(0)));
      }
    } catch (InputSourceException &) { //-V565
      // it`s ok. the error is not interesting
    }
  }

  // /usr/share/colorer/catalog.xml
  paths.emplace_back(SString(CString("/usr/share/colorer/catalog.xml")));
  paths.emplace_back(SString(CString("/usr/local/share/colorer/catalog.xml")));
}
#endif

void ParserFactory::loadCatalog(const String* catalog_path)
{
  if (!catalog_path) {
    base_catalog_path = searchCatalog();
    if (base_catalog_path.length() == 0) {
      throw ParserFactoryException(CString("Can't find suitable catalog.xml file."));
    }
  } else {
    base_catalog_path = SString(catalog_path);
  }


  parseCatalog(base_catalog_path);
  LOG(G3LOG_DEBUG) << "begin load hrc files";
  for (auto location : hrc_locations) {
    try {
      LOG(G3LOG_DEBUG) << "try load '" << location.getChars() << "'";
      auto clear_path = XmlInputSource::getClearPath(&base_catalog_path, &location);
      if (XmlInputSource::isDirectory(clear_path.get())) {
        std::vector<SString> paths;
        XmlInputSource::getFileFromDir(clear_path.get(), paths);
        for (auto files : paths) {
          loadHrc(&files, &base_catalog_path);
        }
      } else {
        loadHrc(clear_path.get(), &base_catalog_path);
      }
    } catch (const Exception &e) {
      LOG(ERROR) << e.what();
    }
  }

  LOG(G3LOG_DEBUG) << "end load hrc files";
}

void ParserFactory::loadHrc(const String* hrc_path, const String* base_path) const
{
  uXmlInputSource dfis = XmlInputSource::newInstance(hrc_path->getWChars(), base_path->getWChars());
  try {
    hrc_parser->loadSource(dfis.get());
  } catch (Exception &e) {
    LOGF(ERROR, "Can't load hrc: %s", XStr(dfis->getInputSource()->getSystemId()).get_char());
    LOG(ERROR) << e.what();
  }
}

void ParserFactory::parseCatalog(const SString &catalog_path)
{
  hrc_locations.clear();
  hrd_nodes.clear();

  CatalogParser catalog_parser;
  catalog_parser.parse(&catalog_path);

  for (auto hrc_location : catalog_parser.hrc_locations) {
    hrc_locations.push_back(hrc_location);
  }

  while (!catalog_parser.hrd_nodes.empty()) {
    auto hrd = std::move(catalog_parser.hrd_nodes.front());
    catalog_parser.hrd_nodes.pop_front();
    if (hrd_nodes.find(hrd->hrd_class) == hrd_nodes.end()) {
      hrd_nodes.emplace(hrd->hrd_class, std::make_unique<std::vector<std::unique_ptr<HRDNode>>>());
    }
    hrd_nodes.at(hrd->hrd_class)->emplace_back(std::move(hrd));
  }
}

const char* ParserFactory::getVersion()
{
  return "Colorer-take5 Library be5 28 May 2006";
}

size_t ParserFactory::countHRD(const String &classID)
{
  auto hash = hrd_nodes.find(classID);
  if (hash == hrd_nodes.end()) {
    return 0;
  }
  return hash->second->size();
}

std::vector<SString> ParserFactory::enumHRDClasses()
{
  std::vector<SString> result;
  result.reserve(hrd_nodes.size());
  for (auto p = hrd_nodes.begin(); p != hrd_nodes.end(); ++p) {
    result.push_back(p->first);
  }
  return result;
}

std::vector<const HRDNode*> ParserFactory::enumHRDInstances(const String &classID)
{
  auto hash = hrd_nodes.find(classID);
  std::vector<const HRDNode*> result;
  result.reserve(hash->second->size());
  for (auto p = hash->second->begin(); p != hash->second->end(); ++p) {
    result.push_back(p->get());
  }
  return result;
}

const HRDNode* ParserFactory::getHRDNode(const String &classID, const String &nameID)
{
  auto hash = hrd_nodes.find(classID);
  if (hash == hrd_nodes.end()) {
    throw ParserFactoryException(SString("can't find HRDClass '") + classID + "'");
  }
  for (auto p = hash->second->begin(); p != hash->second->end(); ++p) {
    if (nameID.compareTo(p->get()->hrd_name) == 0) {
      return p->get();
    }
  }
  throw ParserFactoryException(SString("can't find HRDName '") + nameID + "'");
}

HRCParser* ParserFactory::getHRCParser() const
{
  return hrc_parser;
}

TextParser* ParserFactory::createTextParser()
{
  return new TextParserImpl();
}

StyledHRDMapper* ParserFactory::createStyledMapper(const String* classID, const String* nameID)
{
  const String* class_id;
  const CString class_default("rgb");
  if (classID == nullptr) {
    class_id = &class_default;
  } else {
    class_id = classID;
  }

  const String* name_id;
  const CString name_default("default");
  CString name_env;
  if (nameID == nullptr) {
    char* hrd = getenv("COLORER5HRD");
    if (hrd != nullptr) {
      name_env = CString(hrd);
      name_id = &name_env;
    } else {
      name_id = &name_default;
    }
  } else {
    name_id = nameID;
  }

  auto hrd_node = getHRDNode(*class_id, *name_id);

  StyledHRDMapper* mapper = new StyledHRDMapper();
  for (size_t idx = 0; idx < hrd_node->hrd_location.size(); idx++)
    if (hrd_node->hrd_location.at(idx).length() != 0) {
      uXmlInputSource dfis = nullptr;
      try {
        dfis = XmlInputSource::newInstance(hrd_node->hrd_location.at(idx).getWChars(), base_catalog_path.getWChars());
        mapper->loadRegionMappings(dfis.get());
      } catch (Exception &e) {
        LOGF(ERROR, "Can't load hrd:");
        LOG(ERROR) << e.what();
        throw ParserFactoryException(CString("Error load hrd"));
      }
    }
  return mapper;
}

TextHRDMapper* ParserFactory::createTextMapper(const String* nameID)
{
  // fixed class 'text'
  CString d_text = CString("text");

  const String* name_id;
  const CString name_default("default");
  if (nameID == nullptr) {
    name_id = &name_default;
  } else {
    name_id = nameID;
  }

  auto hrd_node = getHRDNode(d_text, *name_id);

  TextHRDMapper* mapper = new TextHRDMapper();
  for (size_t idx = 0; idx <  hrd_node->hrd_location.size(); idx++)
    if (hrd_node->hrd_location.at(idx).length() != 0) {
      uXmlInputSource dfis = nullptr;
      try {
        dfis = XmlInputSource::newInstance(hrd_node->hrd_location.at(idx).getWChars(), base_catalog_path.getWChars());
        mapper->loadRegionMappings(dfis.get());
      } catch (Exception &e) {
        LOG(ERROR) << "Can't load hrd: ";
        LOG(ERROR) << e.what();
      }
    }
  return mapper;
}

/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Colorer Library.
 *
 * The Initial Developer of the Original Code is
 * Cail Lomecb <irusskih at gmail dot com>.
 * Portions created by the Initial Developer are Copyright (C) 1999-2009
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s): see file CONTRIBUTORS
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

