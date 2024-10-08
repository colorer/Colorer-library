#include "colorer/xml/libxml2/LibXmlInputSource.h"
#include "colorer/Exception.h"
#include "colorer/utils/Environment.h"

LibXmlInputSource::LibXmlInputSource(const UnicodeString& path, const UnicodeString* base)
{
  if (path.isEmpty()) {
    throw InputSourceException("XmlInputSource::newInstance: path is empty");
  }
  UnicodeString full_path;
  if (path.startsWith(jar) || (base != nullptr && base->startsWith(jar))) {
#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
    initZipSource(path, base);
#else
    throw InputSourceException("zip input source not supported");
#endif
  }
  else if (colorer::Environment::isRegularFile(base, &path, full_path)) {
    sourcePath = full_path;
  }
  else {
    throw InputSourceException(full_path + " isn't regular file.");
  }
}

LibXmlInputSource::~LibXmlInputSource()
{
#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
  if (zip_source) {
    zip_source->delref();
  }
#endif
}

LibXmlInputSource LibXmlInputSource::createRelative(const UnicodeString& relPath) const
{
  return LibXmlInputSource(relPath, &sourcePath);
}

UnicodeString& LibXmlInputSource::getPath()
{
  return sourcePath;
}

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
void LibXmlInputSource::initZipSource(const UnicodeString& path, const UnicodeString* base)
{
  auto paths = getFullPathFromPathJar(path, base);

  sourcePath = paths.full_path;
  zip_source = SharedXmlInputSource::getSharedInputSource(paths.path_to_jar);
}

PathInJar LibXmlInputSource::getFullPathFromPathJar(const UnicodeString& path, const UnicodeString* base)
{
  if (path.startsWith(jar)) {
    const auto path_idx = path.lastIndexOf('!');
    if (path_idx == -1) {
      throw InputSourceException("Bad jar uri format: " + path);
    }

    const UnicodeString path_to_jar = colorer::Environment::getAbsolutePath(
        base ? *base : u"", UnicodeString(path, jar.length(), path_idx - jar.length()));
    const UnicodeString path_in_jar(path, path_idx + 1);

    const UnicodeString full_path = jar + path_to_jar + u"!" + path_in_jar;
    return {full_path, path_to_jar, path_in_jar};
  }
  else {
    const auto base_idx = base->lastIndexOf('!');
    if (base_idx == -1) {
      throw InputSourceException("Bad jar uri format: " + path);
    }
    const UnicodeString path_to_jar(*base, jar.length(), base_idx - jar.length());
    const UnicodeString path_in_jar = colorer::Environment::getAbsolutePath(UnicodeString(*base, base_idx + 1), path);

    const UnicodeString full_path = jar + path_to_jar + u"!" + path_in_jar;
    return {full_path, path_to_jar, path_in_jar};
  }
}
#endif
