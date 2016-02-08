#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#ifdef __unix__
#include <dirent.h>
#endif
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#endif

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/BinFileInputStream.hpp>

#include <colorer/ParserFactory.h>
#include <colorer/viewer/TextLinesStore.h>
#include <colorer/parsers/HRCParserImpl.h>
#include <colorer/parsers/TextParserImpl.h>
#include <colorer/ParserFactoryException.h>

#include <xml/XmlInputSource.h>
#include <xml/XmlParserErrorHandler.h>
#include <xml/BaseEntityResolver.h>
#include <xml/XmlTagDefs.h>
#include <xml/XStr.h>

ParserFactory::ParserFactory(): catalogPath(nullptr), catalogXIS(nullptr), hrcParser(nullptr)
{
  RegExpStack = nullptr;
  RegExpStack_Size = 0;
}

ParserFactory::~ParserFactory()
{
  for (auto it : hrcLocations) {
    delete it;
  }
  hrcLocations.clear();

  // Ohhh!
  for (auto it : hrdLocations) {
    for (auto it2 : *it.second) {
      for (auto it3 : *it2.second) {
        delete it3;
      }
      delete it2.second;
    }
    delete it.second;
  }

  for (auto it : hrdDescriptions) {
    delete it.second;
  }
  hrdDescriptions.clear();

  delete hrcParser;
  delete[] RegExpStack;
}

UString ParserFactory::searchCatalog() const
{
  LOG(DEBUG) << "begin search catalog.xml";

  std::vector<SString> paths;
#ifdef _WIN32
  getPossibleCatalogPathsWindows(paths);
#else
  getPossibleCatalogPathsLinux(paths);
#endif

  UString right_path(nullptr);
  for (auto path : paths) {
    try {
      LOG(DEBUG) << "test path '" << path.getChars() << "'";

      uXmlInputSource catalog = XmlInputSource::newInstance(path.getWChars(), static_cast<XMLCh*>(nullptr));
 
      std::unique_ptr<xercesc::BinInputStream> stream(catalog->makeStream());
      right_path.reset(new SString(path));

      LOG(DEBUG) << "found valid path '" << path.getChars() << "'";
      break;
    } catch (const Exception &e) {
      LOG(ERROR) << e.what();
    }
  }
  LOG(DEBUG) << "end search catalog.xml";
  if (!right_path) {
    LOGF(ERRORF, "Can't find suitable catalog.xml file. Check your program settings.");
    throw ParserFactoryException(DString("Can't find suitable catalog.xml file. Check your program settings."));
  }
  return right_path;
}

void ParserFactory::getPossibleCatalogPathsWindows(std::vector<SString> &paths) const
{
  // image_path/  image_path/..
  HMODULE hmod = GetModuleHandle(nullptr);
  if (hmod) {
    wchar_t cname[MAX_PATH];
    int len = GetModuleFileName(hmod, cname, MAX_PATH);
    if (len > 0) {
      DString module(cname, 0, len - 1);
      int pos[2];
      pos[0] = module.lastIndexOf('\\');
      pos[1] = module.lastIndexOf('\\', pos[0]);
      for (int idx = 0; idx < 2; idx++)
        if (pos[idx] >= 0) {
          paths.emplace_back(StringBuffer(DString(module, 0, pos[idx])).append(DString("\\catalog.xml")));
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
      StringBuffer d = StringBuffer(home_drive).append(DString(home_path)).append(DString("/.colorer5catalog"));
      if (_access(d.getChars(), 0) != -1) {
        TextLinesStore tls;
        tls.loadFile(&d, nullptr, false);
        if (tls.getLineCount() > 0) {
          paths.emplace_back(SString(tls.getLine(0)));
        }
      }
    } catch (InputSourceException &) {}
  }
}

void ParserFactory::getPossibleCatalogPathsLinux(std::vector<SString> &paths) const
{
  // %COLORER5CATALOG%
  char* colorer5_catalog = getenv("COLORER5CATALOG");
  if (colorer5_catalog) {
    paths.emplace_back(SString(colorer5_catalog));
  }

  // %HOME%/.colorer5catalog or %HOMEPATH%
  char* home_path = getenv("HOME");
  if (home_path == nullptr) {
    home_path = getenv("HOMEPATH");
  }
  if (home_path != nullptr) {
    try {
      TextLinesStore tls;
      tls.loadFile(&StringBuffer(home_path).append(DString("/.colorer5catalog")), nullptr, false);
      if (tls.getLineCount() > 0) {
        paths.emplace_back(SString(tls.getLine(0)));
      }
    } catch (InputSourceException &) {}
  }

  // /usr/share/colorer/catalog.xml
  paths.emplace_back(SString("/usr/share/colorer/catalog.xml"));
  paths.emplace_back(SString("/usr/local/share/colorer/catalog.xml"));
}

void ParserFactory::loadCatalog(const String* catalogPath_)
{
  if (catalogPath_ == nullptr) {
    catalogPath = searchCatalog();
  } else {
    catalogPath.reset(new SString(catalogPath_));
  }

  xercesc::XercesDOMParser xml_parser;
  XmlParserErrorHandler error_handler;
  BaseEntityResolver resolver;
  xml_parser.setErrorHandler(&error_handler);
  xml_parser.setXMLEntityResolver(&resolver);
  xml_parser.setLoadExternalDTD(false);
  xml_parser.setSkipDTDValidation(true);
  catalogXIS = XmlInputSource::newInstance(catalogPath->getWChars(), static_cast<XMLCh*>(nullptr));
  xml_parser.parse(*catalogXIS->getInputSource());
  if (error_handler.getSawErrors()) {
    throw ParserFactoryException(DString("Error reading catalog.xml."));
  }
  xercesc::DOMDocument* catalog = xml_parser.getDocument();
  xercesc::DOMElement* elem = catalog->getDocumentElement();

  if (elem == nullptr || !xercesc::XMLString::equals(elem->getNodeName(), catTagCatalog)) {
    throw ParserFactoryException(StringBuffer("Bad catalog structure. Main '<catalog>' block not found at file ") + catalogPath.get());
  }
  parseCatalogBlock(elem);
}

void ParserFactory::parseCatalogBlock(const xercesc::DOMElement* elem)
{
  for (xercesc::DOMNode* node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement* subelem = static_cast<xercesc::DOMElement*>(node);
      if (xercesc::XMLString::equals(subelem->getNodeName(), catTagHrcSets)) {
        parseHrcSetsBlock(subelem);
        continue;
      }
      if (xercesc::XMLString::equals(subelem->getNodeName(), catTagHrdSets)) {
        parseHrdSetsBlock(subelem);
      }
      continue;
    }
    if (node->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      parseCatalogBlock(static_cast<xercesc::DOMElement*>(node));
    }
  }
}

void ParserFactory::parseHrcSetsBlock(const xercesc::DOMElement* elem)
{
  // TODO: break catHrcSetsAttrLoglocation
  addHrcSetsLocation(elem);
}

void ParserFactory::addHrcSetsLocation(const xercesc::DOMElement* elem)
{
  for (xercesc::DOMNode* node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement* subelem = static_cast<xercesc::DOMElement*>(node);
      if (xercesc::XMLString::equals(subelem->getNodeName(), catTagLocation)) {
        hrcLocations.push_back(new SString(DString(subelem->getAttribute(catLocationAttrLink))));
      }
      continue;
    }
    if (node->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      addHrcSetsLocation(static_cast<xercesc::DOMElement*>(node));
    }
  }
}

void ParserFactory::parseHrdSetsBlock(const xercesc::DOMElement* elem)
{
  for (xercesc::DOMNode* node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement* subelem = static_cast<xercesc::DOMElement*>(node);
      if (xercesc::XMLString::equals(subelem->getNodeName(), catTagHrd)) {
        parseHRDSetsChild(subelem);
      }
      continue;
    }
    if (node->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      parseHrdSetsBlock(static_cast<xercesc::DOMElement*>(node));
    }
  }
}

void ParserFactory::parseHRDSetsChild(const xercesc::DOMElement* elem)
{
  const XMLCh* xhrd_class = elem->getAttribute(catHrdAttrClass);
  const XMLCh* xhrd_name = elem->getAttribute(catHrdAttrName);
  if (*xhrd_class == '\0' || *xhrd_name == '\0') {
    return;
  }

  const String* hrd_class = new DString(xhrd_class);
  const String* hrd_name = new DString(xhrd_name);
  std::unordered_map<SString, std::vector<const String*>*>* hrdClass;
  auto it_hrdClass = hrdLocations.find(hrd_class);
  if (it_hrdClass == hrdLocations.end()) {
    hrdClass = new std::unordered_map<SString, std::vector<const String*>*>;
    std::pair<SString, std::unordered_map<SString, std::vector<const String*>*>*> pair_class(hrd_class, hrdClass);
    hrdLocations.emplace(pair_class);
  } else {
    hrdClass = it_hrdClass->second;
  }

  auto it_hrdName = hrdClass->find(hrd_name);
  if (it_hrdName != hrdClass->end()) {
    LOGF(ERROR, "Duplicate hrd name '%s'", hrd_name->getChars());
    delete hrd_class;
    delete hrd_name;
    return;
  }

  std::vector<const String*>* hrdLocV(new std::vector<const String*>);
  std::pair<SString, std::vector<const String*>*> pair_name(hrd_name, hrdLocV);
  hrdClass->emplace(pair_name);

  const String* hrd_descr = new SString(DString(elem->getAttribute(catHrdAttrDescription)));
  if (hrd_descr == nullptr) {
    hrd_descr = new SString(hrd_name);
  }
  std::pair<SString, const String*> pp(&(StringBuffer(hrd_class) + "-" + hrd_name), hrd_descr);
  hrdDescriptions.emplace(pp);

  for (xercesc::DOMNode* node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      if (xercesc::XMLString::equals(node->getNodeName(), catTagLocation)) {
        xercesc::DOMElement* subelem = static_cast<xercesc::DOMElement*>(node);
        hrdLocV->push_back(new SString(DString(subelem->getAttribute(catLocationAttrLink))));
      }
    }
  }
  delete hrd_class;
  delete hrd_name;
}

const char* ParserFactory::getVersion()
{
  return "Colorer-take5 Library be5 28 May 2006";
}

size_t ParserFactory::countHRD(const String &classID)
{
  auto hash = hrdLocations.find(classID);
  if (hash == hrdLocations.end()) {
    return 0;
  }
  return hash->second->size();
}

std::vector<SString> ParserFactory::enumHRDClasses()
{
  std::vector<SString> r;
  r.reserve(hrdLocations.size());
  for (auto p = hrdLocations.begin(); p != hrdLocations.end(); ++p) {
    r.push_back(p->first);
  }
  return r;
}

std::vector<SString> ParserFactory::enumHRDInstances(const String &classID)
{
  auto hash = hrdLocations.find(classID);
  std::vector<SString> r;
  r.reserve(hash->second->size());
  for (auto p = hash->second->begin(); p != hash->second->end(); ++p) {
    r.push_back(p->first);
  }
  return r;
}

const String* ParserFactory::getHRDescription(const String &classID, const String &nameID)
{
  auto it = hrdDescriptions.find(&(StringBuffer(classID) + "-" + nameID));
  if (it != hrdDescriptions.end()) {
    return it->second;
  } else {
    return nullptr;
  }
}

HRCParser* ParserFactory::getHRCParser()
{
  if (hrcParser != nullptr) {
    return hrcParser;
  }
  hrcParser = new HRCParserImpl();
  for (size_t idx = 0; idx < hrcLocations.size(); idx++) {
    if (hrcLocations.at(idx) != nullptr) {
#ifdef _WIN32
      size_t i = ExpandEnvironmentStrings(hrcLocations.at(idx)->getWChars(), nullptr, 0);
      wchar_t* temp = new wchar_t[i];
      ExpandEnvironmentStrings(hrcLocations.at(idx)->getWChars(), temp, static_cast<DWORD>(i));
      const String* relPath = new SString(temp);
      delete[] temp;
#else
      const String* relPath = new SString(hrcLocations.at(idx));
#endif
      const String* path;
      if (colorer::InputSource::isRelative(relPath)) {
        path = colorer::InputSource::getAbsolutePath(catalogPath.get(), relPath);
        const String* path2del = path;
        if (path->startsWith(DString("file://"))) {
          path = new SString(path, 7, -1);
        }
        if (path->startsWith(DString("file:/"))) {
          path = new SString(path, 6, -1);
        }
        if (path->startsWith(DString("file:"))) {
          path = new SString(path, 5, -1);
        }
        if (path != path2del) {
          delete path2del;
        }
      } else {
        path = new SString(relPath);
      }

      struct stat st;
      int ret = stat(path->getChars(), &st);

      if (ret != -1 && (st.st_mode & S_IFDIR)) {
#ifdef _WIN32
        loadPathWindows(path, relPath);
#endif
#ifdef __unix__
        loadPathLinux(path, relPath);
#endif
      } else {
        uXmlInputSource dfis = nullptr;
        try {
          dfis = XmlInputSource::newInstance(hrcLocations.at(idx)->getWChars(), catalogXIS.get());
          hrcParser->loadSource(dfis.get());
        } catch (Exception &e) {
          LOG(ERRORF) << "Can't load hrc : " << XStr(dfis->getInputSource()->getSystemId());
          LOG(ERRORF) << e.what();
        }
      }
      delete path;
      delete relPath;
    }
  }
  return hrcParser;
}

void ParserFactory::loadPathWindows(const String* path, const String* relPath)
{
#ifdef _WIN32
  WIN32_FIND_DATA ffd;
  HANDLE dir = FindFirstFile((StringBuffer(path) + "\\*.*").getTChars(), &ffd);
  if (dir != INVALID_HANDLE_VALUE) {
    while (true) {
      if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        uXmlInputSource dfis = XmlInputSource::newInstance((StringBuffer(relPath) + "\\" + SString(ffd.cFileName)).getWChars(), catalogXIS.get());
        try {
          hrcParser->loadSource(dfis.get());
        } catch (Exception &e) {
          LOGF(ERRORF, "Can't load hrc: %s", XStr(dfis->getInputSource()->getSystemId()).get_char());
          LOG(ERRORF) << e.what();
        }
      }
      if (FindNextFile(dir, &ffd) == FALSE) {
        break;
      }
    }
    FindClose(dir);
  }
#endif
}

void ParserFactory::loadPathLinux(const String* path, const String* relPath)
{
#ifdef __unix__
  DIR* dir = opendir(path->getChars());
  dirent* dire;
  if (dir != nullptr) {
    while ((dire = readdir(dir)) != nullptr) {
      struct stat st;
      stat((StringBuffer(path) + "/" + dire->d_name).getChars(), &st);
      if (!(st.st_mode & S_IFDIR)) {
        XmlInputSource* dfis = XmlInputSource::newInstance((StringBuffer(relPath) + "/" + dire->d_name).getWChars(), catalogXIS);
        try {
          hrcParser->loadSource(dfis);
          delete dfis;
        } catch (Exception &e) {
          LOGF(ERRORF, "Can't load hrc: %s", XStr(dfis->getInputSource()->getSystemId()).get_char());
          LOG(ERRORF) << e.getMessage()->getChars();
          delete dfis;
        }
      }
    }
  }
#endif
}

TextParser* ParserFactory::createTextParser()
{
  return new TextParserImpl();
}

StyledHRDMapper* ParserFactory::createStyledMapper(const String* classID, const String* nameID)
{
  const String* class_id;
  const DString class_default("rgb");
  if (classID == nullptr) {
    class_id = &class_default;
  } else {
    class_id = classID;
  }

  auto it_hrdClass = hrdLocations.find(class_id);
  if (it_hrdClass == hrdLocations.end()) {
    throw ParserFactoryException(StringBuffer("can't find hrdClass '") + classID + "'");
  }
  std::unordered_map<SString, std::vector<const String*>*>* hrdClass = it_hrdClass->second;

  const String* name_id;
  const DString name_default("default");
  DString name_env;
  if (nameID == nullptr) {
    char* hrd = getenv("COLORER5HRD");
    if (hrd != nullptr) {
      name_env = DString(hrd);
      name_id = &name_env;
    } else {
      name_id = &name_default;
    }
  } else {
    name_id = nameID;
  }

  auto it_hrdLocV = hrdClass->find(name_id);
  if (it_hrdLocV == hrdClass->end()) {
    throw ParserFactoryException(StringBuffer("can't find hrdName '") + name_id + "'");
  }
  std::vector<const String*>* hrdLocV = it_hrdLocV->second;

  StyledHRDMapper* mapper = new StyledHRDMapper();
  for (size_t idx = 0; idx < hrdLocV->size(); idx++)
    if (hrdLocV->at(idx) != nullptr) {
      uXmlInputSource dfis = nullptr;
      try {
        dfis = XmlInputSource::newInstance(hrdLocV->at(idx)->getWChars(), catalogXIS.get());
        mapper->loadRegionMappings(dfis.get());
      } catch (Exception &e) {
        LOGF(ERROR, "Can't load hrd:");
        LOG(ERROR) << e.what();
        throw ParserFactoryException(DString("Error load hrd"));
      }
    }
  return mapper;
}

TextHRDMapper* ParserFactory::createTextMapper(const String* nameID)
{
  // fixed class 'text'
  DString d_text = DString("text");
  auto it_hrdClass = hrdLocations.find(&d_text);
  if (it_hrdClass == hrdLocations.end()) {
    throw ParserFactoryException(StringBuffer("can't find hrdClass 'text'"));
  }

  std::unordered_map<SString, std::vector<const String*>*>* hrdClass = it_hrdClass->second;
  const String* name_id;
  const DString name_default("default");
  if (nameID == nullptr) {
    name_id = &name_default;
  } else {
    name_id = nameID;
  }

  auto it_hrdLocV = hrdClass->find(name_id);
  if (it_hrdLocV == hrdClass->end()) {
    throw ParserFactoryException(StringBuffer("can't find hrdName '") + name_id + "'");
  }
  std::vector<const String*>* hrdLocV = it_hrdLocV->second;

  TextHRDMapper* mapper = new TextHRDMapper();
  for (size_t idx = 0; idx < hrdLocV->size(); idx++)
    if (hrdLocV->at(idx) != nullptr) {
      uXmlInputSource dfis = nullptr;
      try {
        dfis = XmlInputSource::newInstance(hrdLocV->at(idx)->getWChars(), catalogXIS.get());
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
 * Cail Lomecb <cail@nm.ru>.
 * Portions created by the Initial Developer are Copyright (C) 1999-2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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
