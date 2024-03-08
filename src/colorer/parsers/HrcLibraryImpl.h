#ifndef _COLORER_HRCLIBRARYIMPL_H_
#define _COLORER_HRCLIBRARYIMPL_H_

#include <xercesc/dom/DOM.hpp>
#include "colorer/HrcLibrary.h"
#include "colorer/cregexp/cregexp.h"
#include "colorer/parsers/SchemeImpl.h"
#include "colorer/xml/XmlInputSource.h"

class FileType;

/** Implementation of HrcLibrary.
    Reads and mantains HRC database of syntax rules,
    used by TextParser implementations to make
    realtime text syntax parsing.
    @ingroup colorer_parsers
*/
class HrcLibrary::Impl
{
 public:
  Impl();
  ~Impl();

  void loadSource(XmlInputSource* is);
  void loadFileType(FileType* filetype);
  FileType* getFileType(const UnicodeString* name);
  FileType* enumerateFileTypes(unsigned int index);
  FileType* chooseFileType(const UnicodeString* fileName, const UnicodeString* firstLine,
                           int typeNo = 0);
  size_t getFileTypesCount();

  size_t getRegionCount();
  const Region* getRegion(unsigned int id);
  const Region* getRegion(const UnicodeString* name);

 protected:
  enum class QualifyNameType { QNT_DEFINE, QNT_SCHEME, QNT_ENTITY };

  // types and packages
  std::unordered_map<UnicodeString, FileType*> fileTypeHash;
  // only types
  std::vector<FileType*> fileTypeVector;

  std::unordered_map<UnicodeString, SchemeImpl*> schemeHash;
  std::unordered_map<UnicodeString, int> disabledSchemes;

  std::vector<const Region*> regionNamesVector;
  std::unordered_map<UnicodeString, const Region*> regionNamesHash;
  std::unordered_map<UnicodeString, UnicodeString*> schemeEntitiesHash;

  FileType* current_parse_type = nullptr;
  XmlInputSource* current_input_source = nullptr;
  bool structureChanged = false;
  bool updateStarted = false;

  void unloadFileType(FileType* filetype);

  void parseHRC(const XmlInputSource& is);
  void parseHrcBlock(const xercesc::DOMElement* elem);
  void parseHrcBlockElements(const xercesc::DOMNode* elem);
  void addPrototype(const xercesc::DOMElement* elem);
  void parsePrototypeBlock(const xercesc::DOMElement* elem, FileType* current_parse_prototype);
  void addPrototypeLocation(const xercesc::DOMElement* elem, FileType* current_parse_prototype);
  void addPrototypeDetectParam(const xercesc::DOMElement* elem, FileType* current_parse_prototype);
  void addPrototypeParameters(const xercesc::DOMNode* elem, FileType* current_parse_prototype);
  void addType(const xercesc::DOMElement* elem);
  void parseTypeBlock(const xercesc::DOMNode* elem);
  void addTypeRegion(const xercesc::DOMElement* elem);
  void addTypeEntity(const xercesc::DOMElement* elem);
  void addTypeImport(const xercesc::DOMElement* elem);

  void addScheme(const xercesc::DOMElement* elem);
  void parseSchemeBlock(SchemeImpl* scheme, const xercesc::DOMNode* elem);
  void addSchemeInherit(SchemeImpl* scheme, const xercesc::DOMElement* elem);
  void addSchemeRegexp(SchemeImpl* scheme, const xercesc::DOMElement* elem);
  void addSchemeBlock(SchemeImpl* scheme, const xercesc::DOMElement* elem);
  void parseSchemeKeywords(SchemeImpl* scheme, const xercesc::DOMElement* elem);
  size_t getSchemeKeywordsCount(const xercesc::DOMNode* elem);
  void addKeyword(SchemeNode* scheme_node, const Region* region, const xercesc::DOMElement* elem);
  void loadBlockRegions(SchemeNode* node, const xercesc::DOMElement* elem);
  void loadRegions(SchemeNode* node, const xercesc::DOMElement* elem, bool st);

  uUnicodeString qualifyOwnName(const UnicodeString& name);
  bool checkNameExist(const UnicodeString* name, FileType* parseType, QualifyNameType qntype,
                      bool logErrors);
  UnicodeString* qualifyForeignName(const UnicodeString* name, QualifyNameType qntype,
                                    bool logErrors);

  void updateLinks();
  uUnicodeString useEntities(const UnicodeString* name);
  const Region* getNCRegion(const xercesc::DOMElement* elem, const XMLCh* tag);
  const Region* getNCRegion(const UnicodeString* name, bool logErrors);
  void loopKeywords(const xercesc::DOMNode* elem, const Region* region,
           const std::unique_ptr<SchemeNode>& scheme_node);
};

#endif
