#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#ifdef __unix__
#include<dirent.h>
#endif
#ifdef _WIN32
#include<io.h>
#include<windows.h>
#endif

#include<common/Logging.h>

#include<colorer/ParserFactory.h>
#include<colorer/viewer/TextLinesStore.h>
#include<colorer/handlers/DefaultErrorHandler.h>
#include<colorer/handlers/FileErrorHandler.h>
#include<colorer/parsers/HRCParserImpl.h>
#include<colorer/parsers/TextParserImpl.h>

#include <xml/XmlInputSource.h>
#include <xml/XmlParserErrorHandler.h>
#include <xml/BaseEntityResolver.h>
#include <xml/XmlTagDefs.h>

void ParserFactory::loadCatalog(const String *catalogPath)
{
  if (catalogPath == null){
    this->catalogPath = searchPath();
  }else{
    this->catalogPath = new SString(catalogPath);
  }

  xercesc::XercesDOMParser xml_parser;
  XmlParserErrorHandler error_handler(errorHandler);
  BaseEntityResolver resolver;
  xml_parser.setErrorHandler(&error_handler);
  xml_parser.setXMLEntityResolver(&resolver);
  xml_parser.setLoadExternalDTD(false);
  xml_parser.setSkipDTDValidation(true);
  catalogXIS = XmlInputSource::newInstance(catalogPath->getWChars(), (XMLCh*)nullptr);
  xml_parser.parse(*catalogXIS->getInputSource());
  if (error_handler.getSawErrors()) {
    throw ParserFactoryException(DString("Error reading catalog.xml."));
  }
  xercesc::DOMDocument *catalog = xml_parser.getDocument();
  xercesc::DOMElement *elem = catalog->getDocumentElement();

  if (elem == null || !xercesc::XMLString::equals(elem->getNodeName(), catTagCatalog)) {
    throw ParserFactoryException(StringBuffer("Bad catalog structure. Main '<catalog>' block not found at file ")+catalogPath);
  }
  parseCatalogBlock(elem);
}

void ParserFactory::parseCatalogBlock(const xercesc::DOMElement *elem)
{
  for (xercesc::DOMNode *node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement *subelem = static_cast<xercesc::DOMElement*>(node);
      if (xercesc::XMLString::equals(subelem->getNodeName(), catTagHrcSets)) {
        parseHrcSetsBlock(subelem);
        continue;
      }
      if (xercesc::XMLString::equals(subelem->getNodeName(), catTagHrdSets)) {
        parseHrdSetsBlock(subelem);
      };
      continue;
    }
    if (node->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      parseCatalogBlock(static_cast<xercesc::DOMElement*>(node));
    }
  }
}

void ParserFactory::parseHrcSetsBlock(const xercesc::DOMElement *elem)
{
  if (COLORER_FEATURE_LOGLEVEL > COLORER_FEATURE_LOGLEVEL_QUIET){
    //TODO: change debug loging
    const XMLCh *logLocation = elem->getAttribute(catHrcSetsAttrLoglocation);

    if (*logLocation != '\0'){
      XmlInputSource *dfis = XmlInputSource::newInstance(logLocation, catalogXIS);
      try{
        colorer_logger_set_target((char*)dfis->getInputSource()->getSystemId());
      }catch(Exception &){
      };
      delete dfis;
    };
  }
  addHrcSetsLocation(elem);
}

void ParserFactory::addHrcSetsLocation(const xercesc::DOMElement *elem)
{
  for (xercesc::DOMNode *node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement *elem = static_cast<xercesc::DOMElement*>(node);
      if (xercesc::XMLString::equals(elem->getNodeName(), catTagLocation)) {
        hrcLocations.addElement(new SString(DString(elem->getAttribute(catLocationAttrLink))));
      }
      continue;
    }
    if (node->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      addHrcSetsLocation(static_cast<xercesc::DOMElement*>(node));
    }
  }
}

void ParserFactory::parseHrdSetsBlock(const xercesc::DOMElement *elem)
{
  for (xercesc::DOMNode *node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement *subelem = static_cast<xercesc::DOMElement*>(node);
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

void ParserFactory::parseHRDSetsChild(const xercesc::DOMElement *elem)
{
  const XMLCh *xhrd_class = elem->getAttribute(catHrdAttrClass);
  const XMLCh *xhrd_name = elem->getAttribute(catHrdAttrName);
  if (*xhrd_class == '\0' || *xhrd_name == '\0'){
    return;
  };

  const String *hrd_class = new DString(xhrd_class);
  const String *hrd_name = new DString(xhrd_name);
  Hashtable<Vector<const String*>*> *hrdClass = hrdLocations.get(hrd_class);
  if (hrdClass == null){
    hrdClass = new Hashtable<Vector<const String*>*>;
    hrdLocations.put(hrd_class, hrdClass);
  };
  if (hrdClass->get(hrd_name) != null){
    errorHandler->error(StringBuffer("Duplicate hrd class '")+hrd_name+"'");
    delete hrd_class;
    delete hrd_name;
    return;
  };
  hrdClass->put(hrd_name, new Vector<const String*>);
  Vector<const String*> *hrdLocV = hrdClass->get(hrd_name);
  if (hrdLocV == null){
    hrdLocV = new Vector<const String*>;
    hrdClass->put(hrd_name, hrdLocV);
  };

  const String *hrd_descr = new SString(DString(elem->getAttribute(catHrdAttrDescription)));
  if (hrd_descr == null){
    hrd_descr = new SString(hrd_name);
  }
  hrdDescriptions.put(&(StringBuffer(hrd_class)+"-"+hrd_name), hrd_descr);

  for (xercesc::DOMNode *node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      if (xercesc::XMLString::equals(node->getNodeName(), catTagLocation)) {
        xercesc::DOMElement *subelem = static_cast<xercesc::DOMElement*>(node);
        hrdLocV->addElement(new SString(DString(subelem->getAttribute(catLocationAttrLink))));
      }
    }
  }
  delete hrd_class;
  delete hrd_name;
};

String *ParserFactory::searchPath()
{
  Vector<String*> paths;
  TextLinesStore tls;

#ifdef _WIN32
  searchPathWindows(&paths);
#else
  searchPathLinux(&paths);
#endif

  String *right_path = null;
  for(int i = 0; i < paths.size(); i++){
    String *path = paths.elementAt(i);
    if (right_path == null){
      colorer::InputSource *is = null;
      try{
        is = colorer::InputSource::newInstance(path);
        is->openStream();
        right_path = new SString(path);
        delete is;
      }catch(InputSourceException &){
        delete is;
      };
    };
    delete path;
  };
  if (right_path == null){
    errorHandler->fatalError(DString("Can't find suitable catalog.xml file. Check your program settings."));
    throw ParserFactoryException(DString("Can't find suitable catalog.xml file. Check your program settings."));
  };
  return right_path;

};

void ParserFactory::searchPathWindows(Vector<String*> *paths)
{
#ifdef _WIN32
  // image_path/  image_path/..  image_path/../..
  wchar_t cname[256];
  HMODULE hmod;
  hmod = GetModuleHandle(L"colorer");
#ifdef _WIN64
  if (hmod == null) hmod = GetModuleHandle(L"colorer_x64");
#endif
  if (hmod == null) hmod = GetModuleHandle(null);
  int len = GetModuleFileName(hmod, cname, 256) - 1;
  DString module(cname, 0, len);
  int pos[3];
  pos[0] = module.lastIndexOf('\\');
  pos[1] = module.lastIndexOf('\\', pos[0]);
  pos[2] = module.lastIndexOf('\\', pos[1]);
  for(int idx = 0; idx < 3; idx++)
    if (pos[idx] >= 0)
      paths->addElement(&(new StringBuffer(DString(module, 0, pos[idx])))->append(DString("\\catalog.xml")));

  // %COLORER5CATALOG%
  char *c = getenv("COLORER5CATALOG");
  if (c != null) paths->addElement(new SString(c));
  // %HOMEDRIVE%%HOMEPATH%\.colorer5catalog
  char *b = getenv("HOMEDRIVE");
  c = getenv("HOMEPATH");
  if ((c != null)&&(b != null)){
    try{
      StringBuffer *d=new StringBuffer(b);
      d->append(&StringBuffer(c).append(DString("/.colorer5catalog")));
      if (_access(d->getChars(),0) != -1){
        TextLinesStore tls;
        tls.loadFile(d, null, false);
        if (tls.getLineCount() > 0){
          paths->addElement(new SString(tls.getLine(0)));
        }
      }
      delete d;
    }catch(InputSourceException &){};
  };
  // %SYSTEMROOT%/.colorer5catalog
  c = getenv("SYSTEMROOT");
  if (c == null) c = getenv("WINDIR");
  if (c != null){ 
    try{
      StringBuffer *d=new StringBuffer(c);
      d->append(DString("/.colorer5catalog"));
      if (_access(d->getChars(),0) != -1){
         TextLinesStore tls;
        tls.loadFile(d, null, false);
        if (tls.getLineCount() > 0){
          paths->addElement(new SString(tls.getLine(0)));
        }
      }
    }catch(InputSourceException &){};
  };
#endif
}

void ParserFactory::searchPathLinux(Vector<String*> *paths)
{
#ifdef __unix__
  // %COLORER5CATALOG%
  char *c = getenv("COLORER5CATALOG");
  if (c != null) paths->addElement(new SString(c));

  // %HOME%/.colorer5catalog or %HOMEPATH%
  c = getenv("HOME");
  if (c == null) c = getenv("HOMEPATH");
  if (c != null){
    try{
      TextLinesStore tls;
      tls.loadFile(&StringBuffer(c).append(DString("/.colorer5catalog")), null, false);
      if (tls.getLineCount() > 0) paths->addElement(new SString(tls.getLine(0)));
    }catch(InputSourceException &){};
  };

  // /usr/share/colorer/catalog.xml
  paths->addElement(new SString("/usr/share/colorer/catalog.xml"));
  paths->addElement(new SString("/usr/local/share/colorer/catalog.xml"));
#endif
}

ParserFactory::ParserFactory(colorer::ErrorHandler *_errorHandler){
  RegExpStack = NULL;
  RegExpStack_Size = 0;
  if (_errorHandler) {
    errorHandler = _errorHandler;
    ownErrorHandler = false;
  }else{
    errorHandler = new DefaultErrorHandler();
    ownErrorHandler = true;
  }
  hrcParser = null;
  catalogPath = null;
  catalogXIS = null;
  // инициализация xerces, иначе будут ошибки работы со строками
	xercesc::XMLPlatformUtils::Initialize();
};

ParserFactory::~ParserFactory(){
  for(Hashtable<Vector<const String*>*> *hrdClass = hrdLocations.enumerate();
      hrdClass;
      hrdClass = hrdLocations.next())
  {
    for(Vector<const String*> *hrd_name = hrdClass->enumerate();hrd_name ; hrd_name = hrdClass->next()){
      for (int i=0;i<hrd_name->size();i++) delete hrd_name->elementAt(i);
      delete hrd_name;
    };

    delete hrdClass;
  };
  for (int i=0;i<hrcLocations.size();i++) delete hrcLocations.elementAt(i);
  for (const String* hrdD=hrdDescriptions.enumerate(); hrdD; hrdD=hrdDescriptions.next())
  {    
    delete hrdD;
  }

  delete hrcParser;
  delete catalogPath;
  delete catalogXIS;
  if (ownErrorHandler) delete errorHandler;
  delete[] RegExpStack;
  // закрываем xerces, подчищая память
  xercesc::XMLPlatformUtils::Terminate();
};

const char *ParserFactory::getVersion(){
  return "Colorer-take5 Library be5 28 May 2006";
};

int ParserFactory::countHRD(const String &classID)
{
  Hashtable<Vector<const String*>*> *hash = hrdLocations.get(&classID);
  if (hash == null) return 0;
  return hash->size();
}

const String* ParserFactory::enumerateHRDClasses(int idx){
  return hrdLocations.key(idx);
};
const String* ParserFactory::enumerateHRDInstances(const String &classID, int idx){
  Hashtable<Vector<const String*>*> *hash = hrdLocations.get(&classID);
  if (hash == null) return null;
  return hash->key(idx);
};
const String* ParserFactory::getHRDescription(const String &classID, const String &nameID){
  return hrdDescriptions.get(&(StringBuffer(classID)+"-"+nameID));
};

HRCParser* ParserFactory::getHRCParser(){
  if (hrcParser != null) return hrcParser;
  hrcParser = new HRCParserImpl();
  hrcParser->setErrorHandler(errorHandler);
  for(int idx = 0; idx < hrcLocations.size(); idx++){
    if (hrcLocations.elementAt(idx) != null){
#ifdef _WIN32
      size_t i=ExpandEnvironmentStrings( hrcLocations.elementAt(idx)->getWChars(),NULL,0);
      wchar_t *temp = new wchar_t[i];
      ExpandEnvironmentStrings( hrcLocations.elementAt(idx)->getWChars(),temp,static_cast<DWORD>(i));
      const String *relPath = new SString(temp);
      delete[] temp;
#else
      const String *relPath = new SString(hrcLocations.elementAt(idx));
#endif
      const String *path = null;
      if (colorer::InputSource::isRelative(relPath)){
        path = colorer::InputSource::getAbsolutePath(catalogPath, relPath);
        const String *path2del = path;
        if (path->startsWith(DString("file://"))) path = new SString(path, 7, -1);
        if (path->startsWith(DString("file:/"))) path = new SString(path, 6, -1);
        if (path->startsWith(DString("file:"))) path = new SString(path, 5, -1);
        if (path != path2del) delete path2del;
      }else
        path = new SString(relPath);

      struct stat st;
      int ret = stat(path->getChars(), &st);

      if (ret != -1 && (st.st_mode & S_IFDIR)){
#ifdef _WIN32
        loadPathWindows(path, relPath);
#endif
#ifdef __unix__
        loadPathLinux(path, relPath);
#endif
      }else{
        XmlInputSource *dfis = nullptr;
        try{
          dfis = XmlInputSource::newInstance(hrcLocations.elementAt(idx)->getWChars(), catalogXIS);
          hrcParser->loadSource(dfis);
          delete dfis;
        }catch(Exception &e){
          errorHandler->fatalError(StringBuffer("Can't load hrc: ") + DString(dfis->getInputSource()->getSystemId()));
          errorHandler->fatalError(*e.getMessage());
          delete dfis;
        };
      };
      delete path;
      delete relPath;
    };
  };
  return hrcParser;
};

void ParserFactory::loadPathWindows(const String * path, const String * relPath)
{
#ifdef _WIN32
  WIN32_FIND_DATA ffd;
  HANDLE dir = FindFirstFile((StringBuffer(path)+"\\*.*").getTChars(), &ffd);
  if (dir != INVALID_HANDLE_VALUE){
    while(true){
      if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
        XmlInputSource *dfis = XmlInputSource::newInstance((StringBuffer(relPath)+"\\"+SString(ffd.cFileName)).getWChars(), catalogXIS);
        try{
          hrcParser->loadSource(dfis);
          delete dfis;
        }catch(Exception &e){
          errorHandler->fatalError(StringBuffer("Can't load hrc: ") + DString(dfis->getInputSource()->getSystemId()));
          errorHandler->fatalError(*e.getMessage());
          delete dfis;
        };
      };
      if (FindNextFile(dir, &ffd) == FALSE) break;
    };
    FindClose(dir);
  };
#endif
}

void ParserFactory::loadPathLinux(const String * path, const String * relPath)
{
#ifdef __unix__
  DIR *dir = opendir(path->getChars());
  dirent *dire;
  if (dir != null){
    while((dire = readdir(dir)) != null){
      struct stat st;
      stat((StringBuffer(path)+"/"+dire->d_name).getChars(), &st);
      if (!(st.st_mode & S_IFDIR)){
        XmlInputSource *dfis = XmlInputSource::newInstance((StringBuffer(relPath)+"/"+dire->d_name).getWChars(), catalogXIS);
        try{
          hrcParser->loadSource(dfis);
          delete dfis;
        }catch(Exception &e){
          errorHandler->fatalError(StringBuffer("Can't load hrc: ") + DString(dfis->getInputSource()->getSystemId()));
          errorHandler->fatalError(*e.getMessage());
          delete dfis;
        }
      }
    }
  }
#endif
}

TextParser *ParserFactory::createTextParser(){
  return new TextParserImpl();
};

StyledHRDMapper *ParserFactory::createStyledMapper(const String *classID, const String *nameID)
{
  Hashtable<Vector<const String*>*> *hrdClass = null;
  if (classID == null)
    hrdClass = hrdLocations.get(&DString("rgb"));
  else
    hrdClass = hrdLocations.get(classID);

  if (hrdClass == null)
    throw ParserFactoryException(StringBuffer("can't find hrdClass '")+classID+"'");

  Vector<const String*> *hrdLocV = null;
  if (nameID == null)
  {
    char *hrd = getenv("COLORER5HRD");
    hrdLocV = (hrd) ? hrdClass->get(&DString(hrd)) : hrdClass->get(&DString("default"));
    if(hrdLocV == null)
    {
      hrdLocV = hrdClass->enumerate();
    }
  }
  else
    hrdLocV = hrdClass->get(nameID);
  if (hrdLocV == null)
    throw ParserFactoryException(StringBuffer("can't find hrdName '")+nameID+"'");

  StyledHRDMapper *mapper = new StyledHRDMapper();
  for(int idx = 0; idx < hrdLocV->size(); idx++)
    if (hrdLocV->elementAt(idx) != null){
      XmlInputSource *dfis = null;
      try{
        dfis = XmlInputSource::newInstance(hrdLocV->elementAt(idx)->getWChars(), catalogXIS);
        mapper->loadRegionMappings(dfis, errorHandler);
        delete dfis;
      }catch(Exception &e){
        errorHandler->error(DString("Can't load hrd: "));
        errorHandler->error(*e.getMessage());
        delete dfis;
        throw ParserFactoryException(DString("Error load hrd"));
      };
    };
  return mapper;
};

TextHRDMapper *ParserFactory::createTextMapper(const String *nameID){
  // fixed class 'text'
  Hashtable<Vector<const String*>*> *hrdClass = hrdLocations.get(&DString("text"));
  if (hrdClass == null) throw ParserFactoryException(StringBuffer("can't find hrdClass 'text'"));

  Vector<const String*> *hrdLocV = null;
  if (nameID == null)
    hrdLocV = hrdClass->get(&DString("default"));
  else
    hrdLocV = hrdClass->get(nameID);
  if (hrdLocV == null)
    throw ParserFactoryException(StringBuffer("can't find hrdName '")+nameID+"'");

  TextHRDMapper *mapper = new TextHRDMapper();
  for(int idx = 0; idx < hrdLocV->size(); idx++)
    if (hrdLocV->elementAt(idx) != null){
      XmlInputSource *dfis = null;
      try{
        dfis = XmlInputSource::newInstance(hrdLocV->elementAt(idx)->getWChars(), catalogXIS);
        mapper->loadRegionMappings(dfis, errorHandler);
        delete dfis;
      }catch(Exception &e){
        errorHandler->error(DString("Can't load hrd: "));
        errorHandler->error(*e.getMessage());
        delete dfis;
      };
    };
  return mapper;
};

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
