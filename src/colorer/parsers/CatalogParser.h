#ifndef _COLORER_CATALOGPARSER_H_
#define _COLORER_CATALOGPARSER_H_

#include <colorer/Common.h>
#include <colorer/Exception.h>
#include <colorer/parsers/HRDNode.h>
#include <list>
#include <vector>
#include <xercesc/dom/DOM.hpp>

class CatalogParser
{
 public:
  CatalogParser() = default;
  ~CatalogParser() = default;

  void parse(const UnicodeString* path);
  static std::unique_ptr<HRDNode> parseHRDSetsChild(const xercesc::DOMElement* elem);

  std::vector<UnicodeString> hrc_locations;
  std::list<std::unique_ptr<HRDNode>> hrd_nodes;

  CatalogParser(CatalogParser const&) = delete;
  CatalogParser& operator=(CatalogParser const&) = delete;
  CatalogParser(CatalogParser&&) = delete;
  CatalogParser& operator=(CatalogParser&&) = delete;

 private:
  void parseCatalogBlock(const xercesc::DOMElement* elem);
  void parseHrcSetsBlock(const xercesc::DOMElement* elem);
  void addHrcSetsLocation(const xercesc::DOMElement* elem);
  void parseHrdSetsBlock(const xercesc::DOMElement* elem);
};

class CatalogParserException : public Exception
{
 public:
  explicit CatalogParserException(const UnicodeString& msg) noexcept : Exception("[CatalogParserException] " + msg) {}
};

#endif  //_COLORER_CATALOGPARSER_H_
