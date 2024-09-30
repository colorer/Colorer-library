#include "colorer/xml/XmlInputSource.h"

XmlInputSource::XmlInputSource(const UnicodeString& source_path) : XmlInputSource(source_path, nullptr) {}

XmlInputSource::XmlInputSource(const UnicodeString& source_path, const UnicodeString* source_base)
{
  xml_input_source = std::make_unique<LibXmlInputSource>(source_path, source_base);
}

uXmlInputSource XmlInputSource::createRelative(const UnicodeString& relPath) const
{
  auto str = UnicodeString(xml_input_source->getPath());
  return std::make_unique<XmlInputSource>(relPath, &str);
}

UnicodeString& XmlInputSource::getPath() const
{
  return xml_input_source->getPath();
}

bool XmlInputSource::isFileURI(const UnicodeString& path, const UnicodeString* base)
{
  const UnicodeString jar(u"jar:");
  if (path.startsWith(jar) || (base && base->startsWith(jar))) {
    return false;
  }
  return true;
}