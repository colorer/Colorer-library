#include <xml/XmlInputSource.h>
#include <xml/LocalFileXmlInputSource.h>
#include <common/Exception.h>
#include <xercesc/util/XMLString.hpp>
#include <xml/ZipXmlInputSource.h>

XmlInputSource *XmlInputSource::newInstance(const XMLCh *path, XmlInputSource *base)
{
  if (xercesc::XMLString::startsWith(path,kJar)){
    return new ZipXmlInputSource(path, base);
  };
  if (base != nullptr){
    XmlInputSource *is = base->createRelative(path);
    if (is != nullptr) return is;
    throw InputSourceException(DString("Unknown input source type"));
  };
  return new LocalFileXmlInputSource(path, nullptr);
}

XmlInputSource *XmlInputSource::newInstance(const XMLCh *path, const XMLCh *base)
{
  if ((path == nullptr) || (*path == '\0')){
    throw Exception(DString("XmlInputSource::newInstance: path is nullptr"));
  }
  if (xercesc::XMLString::startsWith(path,kJar)){
    return new ZipXmlInputSource(path, base);
  };
  if (base!=nullptr && xercesc::XMLString::startsWith(base,kJar)){
    return new ZipXmlInputSource(path,base,true);
  }
  return new LocalFileXmlInputSource(path, base);
}

String *XmlInputSource::getAbsolutePath(const String*basePath, const String*relPath){
  int root_pos = basePath->lastIndexOf('/');
  int root_pos2 = basePath->lastIndexOf('\\');
  if (root_pos2 > root_pos) root_pos = root_pos2;
  if (root_pos == -1) root_pos = 0;
  else root_pos++;
  StringBuffer *newPath = new StringBuffer();
  newPath->append(DString(basePath, 0, root_pos)).append(relPath);
  return newPath;
};
