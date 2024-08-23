#include "colorer/xml/XmlInputSource.h"

XmlInputSource::XmlInputSource(const UnicodeString& source_path) : XmlInputSource(source_path, nullptr) {}

XmlInputSource::~XmlInputSource()
{
  // xml_input_source child of xercesc classes and need to free before xercesc
  // xml_input_source.reset();
  // xercesc::XMLPlatformUtils::Terminate();
}

XmlInputSource::XmlInputSource(const UnicodeString& source_path, const UnicodeString* source_base)
{
  // xercesc::XMLPlatformUtils::Initialize();
  // xml_input_source = XercesXmlInputSource::newInstance(&source_path, source_base);
  libxml_input_source = std::make_unique<LibXmlInputSource>(&source_path, source_base);
}

uXmlInputSource XmlInputSource::createRelative(const UnicodeString& relPath) const
{
  // auto str = UnicodeString(xml_input_source->getPath());
  auto str = UnicodeString(libxml_input_source->getPath());
   return std::make_unique<XmlInputSource>(relPath, &str);
}

UnicodeString& XmlInputSource::getPath() const
{
  //return xml_input_source->getPath();
  return libxml_input_source->getPath();
}

XercesXmlInputSource* XmlInputSource::getInputSource() const
{
  return xml_input_source.get();
}

bool XmlInputSource::isFileURI(const UnicodeString& path, const UnicodeString* base)
{
  return XercesXmlInputSource::isUriFile(path, base);
}