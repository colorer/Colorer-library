#include <colorer/Exception.h>
#include <colorer/common/UStr.h>
#include <colorer/xml/LocalFileXmlInputSource.h>
#if COLORER_FEATURE_JARINPUTSOURCE
#include <colorer/xml/ZipXmlInputSource.h>
#endif
#include <colorer/utils/Environment.h>
#include <filesystem>

uXmlInputSource XmlInputSource::newInstance(const UnicodeString* path, const UnicodeString* base)
{
  return newInstance(UStr::to_xmlch(path).get(), UStr::to_xmlch(base).get());
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

uUnicodeString XmlInputSource::getClearFilePath(const UnicodeString* basePath, const UnicodeString* relPath)
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
  if ((path->startsWith(kJar)) || (base && base->startsWith(kJar))) {
    return false;
  }
  return true;
}

uXmlInputSource XmlInputSource::createRelative(const XMLCh* relPath)
{
  return newInstance(relPath, this->getInputSource()->getSystemId());
}
