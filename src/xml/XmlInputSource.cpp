#include <xml/XmlInputSource.h>
#include <xml/LocalFileXmlInputSource.h>
#include <xml/ZipXmlInputSource.h>
#include <common/Exception.h>
#include <xercesc/util/XMLString.hpp>

uXmlInputSource XmlInputSource::newInstance(const XMLCh* path, XmlInputSource* base)
{
  if (xercesc::XMLString::startsWith(path, kJar)) {
    return std::make_unique<ZipXmlInputSource>(path, base);
  }
  if (base) {
    return base->createRelative(path);
  }
  return std::make_unique<LocalFileXmlInputSource>(path, nullptr);
}

uXmlInputSource XmlInputSource::newInstance(const XMLCh* path, const XMLCh* base)
{
  if (!path || (*path == '\0')) {
    throw InputSourceException("XmlInputSource::newInstance: path is nullptr");
  }
  if (xercesc::XMLString::startsWith(path, kJar) || (base != nullptr && xercesc::XMLString::startsWith(base, kJar))) {
    return std::make_unique<ZipXmlInputSource>(path, base);
  }
  return std::make_unique<LocalFileXmlInputSource>(path, base);
}

std::unique_ptr<String> XmlInputSource::getAbsolutePath(const String* basePath, const String* relPath)
{
  int root_pos = basePath->lastIndexOf('/');
  int root_pos2 = basePath->lastIndexOf('\\');
  if (root_pos2 > root_pos) {
    root_pos = root_pos2;
  }
  if (root_pos == -1) {
    root_pos = 0;
  } else {
    root_pos++;
  }
  std::unique_ptr<StringBuffer> newPath(new StringBuffer());
  newPath->append(DString(basePath, 0, root_pos)).append(relPath);
  return std::move(newPath);
}
