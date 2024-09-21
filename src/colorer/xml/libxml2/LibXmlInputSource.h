#ifndef COLORER_LIBXMLINPUTSOURCE_H
#define COLORER_LIBXMLINPUTSOURCE_H

#include "colorer/Common.h"
#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
#include "colorer/xml/libxml2/SharedXmlInputSource.h"
#endif

static const UnicodeString jar(u"jar:");

struct PathInJar
{
  UnicodeString full_path;
  UnicodeString path_to_jar;
  UnicodeString path_in_jar;
};

class LibXmlInputSource
{
 public:
  explicit LibXmlInputSource(const UnicodeString& path, const UnicodeString* base = nullptr);
  ~LibXmlInputSource();

  LibXmlInputSource createRelative(const UnicodeString& relPath) const;

  [[nodiscard]]
  UnicodeString& getPath();

 private:
  UnicodeString sourcePath;

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
 public:
  void initZipSource(const UnicodeString& path, const UnicodeString* base = nullptr);

  static PathInJar getFullPathFromPathJar(const UnicodeString& path, const UnicodeString* base);

 private:
  SharedXmlInputSource* zip_source {nullptr};
#endif
};

#endif  // COLORER_LIBXMLINPUTSOURCE_H
