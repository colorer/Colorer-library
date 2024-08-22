#ifndef COLORER_XERCESXML_H
#define COLORER_XERCESXML_H

#include <list>
#include <unordered_map>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include "colorer/Common.h"
#include "colorer/xml/XMLNode.h"

class XercesXmlReader
{
 public:
  bool saw_error = false;

  explicit XercesXmlReader(const xercesc::InputSource* in);

  void parse(std::list<XMLNode>& nodes);

  ~XercesXmlReader();

 private:
  bool populateNode(const xercesc::DOMNode* node, XMLNode& result);
  void getAttributes(const xercesc::DOMElement* node, std::unordered_map<UnicodeString, UnicodeString>& data);
  void getChildren(const xercesc::DOMNode* node, XMLNode& result);
  const XMLCh* getElementText(const xercesc::DOMElement* blkel) const;

  xercesc::XercesDOMParser* xml_parser;
};
#endif  // COLORER_XERCESXML_H
