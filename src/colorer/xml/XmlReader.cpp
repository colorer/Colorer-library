#include "colorer/xml/XmlReader.h"
#ifndef COLORER_FEATURE_LIBXML
#include "colorer/xml/xercesc/XercesXmlReader.h"
#endif

XmlReader::XmlReader(const XmlInputSource& xml_input_source)
{
  input_source = &xml_input_source;
}

XmlReader::~XmlReader()
{
  delete xml_reader;
}

bool XmlReader::parse()
{
#ifdef COLORER_FEATURE_LIBXML
  xml_reader = new LibXmlReader(input_source->getPath());
  return true; // TODO
#else
  xml_reader = new XercesXmlReader(input_source->getInputSource());
  return !xml_reader->saw_error;
#endif
}

void XmlReader::getNodes(std::list<XMLNode>& nodes) const
{
  xml_reader->parse( nodes);
}