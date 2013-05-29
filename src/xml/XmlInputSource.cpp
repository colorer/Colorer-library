#include <xml/XmlInputSource.h>
#include <common/Exception.h>

xercesc::InputSource *XmlInputSource::newInstance(const XMLCh *path, xercesc::InputSource *base)
{
  if (path == nullptr){
    throw Exception(DString("InputSource::newInstance: path is null"));
  }
  if (base != null){
    return new xercesc::LocalFileInputSource(base->getSystemId(), path);
  };
  return new xercesc::LocalFileInputSource(path);
}