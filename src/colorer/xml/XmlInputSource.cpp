#include <colorer/Exception.h>
#include <colorer/common/UStr.h>
#include <colorer/xml/LocalFileXmlInputSource.h>
#if COLORER_FEATURE_JARINPUTSOURCE
#include <colorer/xml/ZipXmlInputSource.h>
#endif
#include <colorer/utils/Environment.h>
#include <filesystem>

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

uXmlInputSource XmlInputSource::newInstance(const UnicodeString* path)
{
  return newInstance(UStr::to_xmlch(path).get(), static_cast<XMLCh*>(nullptr));
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
  auto basep = std::filesystem::path(UStr::to_stdstr(basePath));
  auto relp = std::filesystem::path(UStr::to_stdstr(relPath));
  basep = basep.parent_path() / relp;

  auto newPath = std::make_unique<UnicodeString>(basep.c_str());
  return newPath;
}

bool XmlInputSource::isRelative(const UnicodeString* path)
{
  return std::filesystem::path(UStr::to_stdstr(path)).is_relative();
}

uUnicodeString XmlInputSource::getClearPath(const UnicodeString* basePath, const UnicodeString* relPath)
{
  auto clear_path = Environment::expandEnvironment(relPath);
  if (isRelative(clear_path.get())) {
    clear_path = getAbsolutePath(basePath, clear_path.get());
    if (clear_path->startsWith("file://")) {
      clear_path = std::make_unique<UnicodeString>(*clear_path, 7);
    }
  }
  return clear_path;
}

bool XmlInputSource::isDirectory(const UnicodeString* path)
{
  return std::filesystem::is_directory(UStr::to_stdstr(path));
}

void XmlInputSource::getFileFromDir(const UnicodeString* relPath, std::vector<UnicodeString>& files)
{
  for (auto& p : std::filesystem::directory_iterator(UStr::to_stdstr(relPath))) {
    files.emplace_back(UnicodeString(p.path().c_str()));
  }
}
