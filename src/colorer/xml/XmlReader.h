#ifndef COLORER_XMLREADER_H
#define COLORER_XMLREADER_H

#include "colorer/xml/XMLNode.h"
#include "colorer/xml/XmlInputSource.h"
#include "colorer/xml/xercesc/XercesXmlReader.h"

class XmlReader
{
 public:
  explicit XmlReader(const XmlInputSource& xml_input_source);
  ~XmlReader();
  bool parse();
  void getNodes(std::list<XMLNode>& nodes) const;

 private:
  const XmlInputSource* input_source;
  XercesXmlReader* xerces_reader = nullptr;
};

#endif  // COLORER_XMLREADER_H
