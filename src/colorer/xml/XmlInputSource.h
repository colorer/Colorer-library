#ifndef COLORER_XMLINPUTSOURCE_H
#define COLORER_XMLINPUTSOURCE_H

#include "colorer/Common.h"
#include "colorer/xml/xercesc/XercesXmlInputSource.h"
#include "colorer/xml/libxml2/LibXmlInputSource.h"

class XmlInputSource;
using uXmlInputSource = std::unique_ptr<XmlInputSource>;

class XmlInputSource
{
 public:
  explicit XmlInputSource(const UnicodeString& source_path);
  ~XmlInputSource();

  XmlInputSource(const UnicodeString& source_path, const UnicodeString* source_base);

  [[nodiscard]]
  uXmlInputSource createRelative(const UnicodeString& relPath) const;

  [[nodiscard]]
  UnicodeString& getPath() const;

  [[nodiscard]]
  XercesXmlInputSource* getInputSource() const;

  static bool isFileURI(const UnicodeString& path, const UnicodeString* base);

 private:
  uXercesXmlInputSource xml_input_source;
  std::unique_ptr<LibXmlInputSource> libxml_input_source;
};

#endif  // COLORER_XMLINPUTSOURCE_H
