#include <colorer/Exception.h>
#include <colorer/common/UStr.h>
#include <colorer/xml/LocalFileXmlInputSource.h>
#if COLORER_FEATURE_JARINPUTSOURCE
#include <colorer/xml/ZipXmlInputSource.h>
#endif
#include <colorer/utils/Environment.h>
#include <filesystem>
#include "XmlInputSource.h"

uXmlInputSource XmlInputSource::newInstance(const XMLCh* path, XmlInputSource* base)
{
  if (xercesc::XMLString::startsWith(path, kJar)) {
#if COLORER_FEATURE_JARINPUTSOURCE
    return std::make_unique<ZipXmlInputSource>(path, base);
#else
    throw InputSourceException("ZipXmlInputSource not supported");
#endif
  }
  if (base) {
    return base->createRelative(path);
  }
  return std::make_unique<LocalFileXmlInputSource>(path, nullptr);
}

uXmlInputSource XmlInputSource::newInstance(const UnicodeString* path, const UnicodeString* base)
{
  if (base) {
    return newInstance(UStr::to_xmlch(path).get(), UStr::to_xmlch(base).get());
  } else {
    return newInstance(UStr::to_xmlch(path).get(), static_cast<XMLCh*>(nullptr));
  }
}

uXmlInputSource XmlInputSource::newInstance(const XMLCh* path, const XMLCh* base)
{
  if (!path || (*path == '\0')) {
    throw InputSourceException("XmlInputSource::newInstance: path is empty");
  }
  if (xercesc::XMLString::startsWith(path, kJar) || (base != nullptr && xercesc::XMLString::startsWith(base, kJar))) {
#if COLORER_FEATURE_JARINPUTSOURCE
    return std::make_unique<ZipXmlInputSource>(path, base);
#else
    throw InputSourceException("ZipXmlInputSource not supported");
#endif
  }
  return std::make_unique<LocalFileXmlInputSource>(path, base);
}

uUnicodeString XmlInputSource::getAbsolutePath(const UnicodeString* basePath, const UnicodeString* relPath)
{
  auto root_pos = basePath->lastIndexOf('/');
  auto root_pos2 = basePath->lastIndexOf('\\');
  if (root_pos2 > root_pos) {
    root_pos = root_pos2;
  }
  if (root_pos == -1) {
    root_pos = 0;
  } else {
    root_pos++;
  }
  auto newPath = std::make_unique<UnicodeString>();
  newPath->append(UnicodeString(*basePath, 0, root_pos)).append(*relPath);
  return newPath;
}

uUnicodeString XmlInputSource::getClearPath(const UnicodeString* basePath, const UnicodeString* relPath)
{
  std::filesystem::path fs_basepath;
  if (basePath) {
    auto clear_basepath = Environment::normalizePath(basePath);
    fs_basepath = std::filesystem::path(UStr::to_stdstr(clear_basepath)).parent_path();
  }
  auto clear_relpath = Environment::expandEnvironment(relPath);

  std::filesystem::path full_path;
  if (fs_basepath.empty()) {
    full_path = UStr::to_stdstr(clear_relpath);
  } else {
    full_path = fs_basepath / UStr::to_stdstr(clear_relpath);
  }

  full_path = full_path.lexically_normal();

  return std::make_unique<UnicodeString>(full_path.c_str());
}

bool XmlInputSource::isUriFile(const UnicodeString* path, const UnicodeString* base)
{
  if ((path->startsWith(u"jar:")) || (base && base->startsWith(u"jar:"))) {
    return false;
  }
  return true;
}
