#include <xml/XmlInputSource.h>
#include <xml/LocalFileXmlInputSource.h>
#include <xml/ZipXmlInputSource.h>
#include <common/Exception.h>
#include <xercesc/util/XMLString.hpp>
#ifdef _WIN32
#include<windows.h>
#endif

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

UString XmlInputSource::getAbsolutePath(const String* basePath, const String* relPath)
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

XMLCh* XmlInputSource::ExpandEnvironment(const XMLCh* path)
{
#ifdef _WIN32
  size_t i = ExpandEnvironmentStrings(path, nullptr, 0);
  XMLCh* temp = new XMLCh[i];
  ExpandEnvironmentStrings(path, temp, static_cast<DWORD>(i));
  return temp;
#else
  //TODO реализовать под nix
  XMLSize_t i = xercesc::XMLString::stringLen(path);
  XMLCh* temp = new XMLCh[i];
  xercesc::XMLString::copyString(temp, path);
  return temp;
#endif
}

bool XmlInputSource::isRelative(const String* path)
{
  if (path->indexOf(':') != -1 && path->indexOf(':') < 10) return false;
  if (path->indexOf('/') == 0 || path->indexOf('\\') == 0) return false;
  return true;
}

UString XmlInputSource::getClearPath(const String* basePath, const String* relPath)
{
  UString clear_path(new SString(relPath));
  if (relPath->indexOf(DString("%")) != -1) {
    XMLCh* e_path = ExpandEnvironment(clear_path.get()->getWChars());
    clear_path.reset(new SString(DString(e_path)));
    delete e_path;
  }
  if (isRelative(clear_path.get())) {
    clear_path = std::move(getAbsolutePath(basePath, clear_path.get()));
    if (clear_path->startsWith(DString("file://"))) {
      clear_path.reset(new SString(clear_path.get(), 7, -1));
    }
  }
  return std::move(clear_path);
}
