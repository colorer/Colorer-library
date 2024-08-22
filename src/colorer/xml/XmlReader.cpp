#include "colorer/xml/XmlReader.h"
#include "colorer/xml/xercesc/XercesXmlReader.h"

XmlReader::XmlReader(const XmlInputSource& xml_input_source)
{
  input_source = &xml_input_source;
}

XmlReader::~XmlReader()
{
  delete xerces_reader;
}

bool XmlReader::parse()
{
  xerces_reader = new XercesXmlReader(input_source->getInputSource());
  return !xerces_reader->saw_error;
}

void XmlReader::getNodes(std::list<XMLNode>& nodes) const
{
  xerces_reader->parse(nodes);
}