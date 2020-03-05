#ifndef _COLORER_CATALOGPARSER_H_
#define _COLORER_CATALOGPARSER_H_

#include <list>
#include <vector>
#include <colorer/Common.h>
#include <colorer/parsers/HRDNode.h>
#include <xercesc/dom/DOM.hpp>

class CatalogParser
{
public:
  CatalogParser() {};
  ~CatalogParser() {};

  void parse(const UnicodeString* path);
  static std::unique_ptr<HRDNode> parseHRDSetsChild(const xercesc::DOMElement* elem);

  std::vector<UnicodeString> hrc_locations;
  std::list<std::unique_ptr<HRDNode>> hrd_nodes;
private:

  void parseCatalogBlock(const xercesc::DOMElement* elem);
  void parseHrcSetsBlock(const xercesc::DOMElement* elem);
  void addHrcSetsLocation(const xercesc::DOMElement* elem);
  void parseHrdSetsBlock(const xercesc::DOMElement* elem);

  CatalogParser(CatalogParser const &) = delete;
  CatalogParser &operator=(CatalogParser const &) = delete;
  CatalogParser(CatalogParser &&) = delete;
  CatalogParser &operator=(CatalogParser &&) = delete;
};

class CatalogParserException : public Exception
{
 public:
  explicit CatalogParserException(const UnicodeString& msg) noexcept : Exception("[CatalogParserException] " + msg) {}
};

#endif //_COLORER_CATALOGPARSER_H_


