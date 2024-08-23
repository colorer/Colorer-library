#include "colorer/xml/XmlReader.h"
#include "colorer/xml/xercesc/XercesXmlReader.h"

XmlReader::XmlReader(const XmlInputSource& xml_input_source)
{
  input_source = &xml_input_source;
}

XmlReader::~XmlReader()
{
  delete xerces_reader;
  delete xml_reader;
}

bool XmlReader::parse()
{
  xml_reader = new LibXmlReader(input_source->getPath());
  return true;
  // xerces_reader = new XercesXmlReader(input_source->getInputSource());
  // return !xerces_reader->saw_error;
}

void XmlReader::getNodes(std::list<XMLNode>& nodes) const
{
  //xerces_reader->parse(nodes);
  xml_reader->parse( nodes);
}