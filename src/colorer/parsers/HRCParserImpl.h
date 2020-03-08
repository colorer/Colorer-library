#ifndef _COLORER_HRCPARSERIMPL_H_
#define _COLORER_HRCPARSERIMPL_H_

#include <colorer/cregexp/cregexp.h>
#include <colorer/HRCParser.h>
#include <colorer/parsers/SchemeImpl.h>

#include <xercesc/dom/DOM.hpp>
#include <colorer/xml/XmlInputSource.h>

class FileTypeImpl;

/** Implementation of HRCParser.
    Reads and mantains HRC database of syntax rules,
    used by TextParser implementations to make
    realtime text syntax parsing.
    @ingroup colorer_parsers
*/
class HRCParserImpl : public HRCParser
{
public:
  HRCParserImpl();
  ~HRCParserImpl() override;


  void loadSource(XmlInputSource* is) override;
  FileType* getFileType(const UnicodeString* name) override;
  FileType* enumerateFileTypes(unsigned int index) override;
  FileType* chooseFileType(const UnicodeString* fileName, const UnicodeString* firstLine, int typeNo = 0) override;
  size_t getFileTypesCount() override;

  size_t getRegionCount() override;
  const Region* getRegion(unsigned int id) override;
  const Region* getRegion(const UnicodeString* name) override;

  const UnicodeString* getVersion() override;

protected:
  friend class FileTypeImpl;

  enum QualifyNameType { QNT_DEFINE, QNT_SCHEME, QNT_ENTITY };

  // types and packages
  std::unordered_map<UnicodeString, FileTypeImpl*> fileTypeHash;
  // types, not packages
  std::vector<FileTypeImpl*>    fileTypeVector;

  std::unordered_map<UnicodeString, SchemeImpl*>   schemeHash;
  std::unordered_map<UnicodeString, int> disabledSchemes;

  std::vector<const Region*> regionNamesVector;
  std::unordered_map<UnicodeString, const Region*> regionNamesHash;
  std::unordered_map<UnicodeString, UnicodeString*> schemeEntitiesHash;

  UnicodeString* versionName;

  FileTypeImpl* parseProtoType;
  FileTypeImpl* parseType;
  XmlInputSource* current_input_source;
  bool structureChanged;
  bool updateStarted;

  void loadFileType(FileType* filetype);
  void unloadFileType(FileTypeImpl* filetype);

  void parseHRC(XmlInputSource* is);
  void parseHrcBlock(const xercesc::DOMElement* elem);
  void parseHrcBlockElements(const xercesc::DOMElement* elem);
  void addPrototype(const xercesc::DOMElement* elem);
  void parsePrototypeBlock(const xercesc::DOMElement* elem);
  void addPrototypeLocation(const xercesc::DOMElement* elem);
  void addPrototypeDetectParam(const xercesc::DOMElement* elem);
  void addPrototypeParameters(const xercesc::DOMElement* elem);
  void addType(const xercesc::DOMElement* elem);
  void parseTypeBlock(const xercesc::DOMElement* elem);
  void addTypeRegion(const xercesc::DOMElement* elem);
  void addTypeEntity(const xercesc::DOMElement* elem);
  void addTypeImport(const xercesc::DOMElement* elem);

  void addScheme(const xercesc::DOMElement* elem);
  void parseSchemeBlock(SchemeImpl* scheme, const xercesc::DOMElement* elem);
  void addSchemeInherit(SchemeImpl* scheme, const xercesc::DOMElement* elem);
  void addSchemeRegexp(SchemeImpl* scheme, const xercesc::DOMElement* elem);
  void addSchemeBlock(SchemeImpl* scheme, const xercesc::DOMElement* elem);
  void addSchemeKeywords(SchemeImpl* scheme, const xercesc::DOMElement* elem);
  int getSchemeKeywordsCount(const xercesc::DOMElement* elem);
  void addKeyword(SchemeNode* scheme_node, const Region* brgn, const xercesc::DOMElement* elem);
  void loadBlockRegions(SchemeNode* node, const xercesc::DOMElement* elem);
  void loadRegions(SchemeNode* node, const xercesc::DOMElement* elem, bool st);

  UnicodeString* qualifyOwnName(const UnicodeString* name);
  bool checkNameExist(const UnicodeString* name, FileTypeImpl* parseType, QualifyNameType qntype, bool logErrors);
  UnicodeString* qualifyForeignName(const UnicodeString* name, QualifyNameType qntype, bool logErrors);

  void updateLinks();
  UnicodeString* useEntities(const UnicodeString* name);
  const Region* getNCRegion(const xercesc::DOMElement* elem, const XMLCh* tag);
  const Region* getNCRegion(const UnicodeString* name, bool logErrors);
};

#include<colorer/parsers/FileTypeImpl.h>

#endif



