#include <xml/LocalFileXmlInputSource.h>
#include <xercesc/util/BinFileInputStream.hpp>
#include <xercesc/util/XMLString.hpp>
#ifdef _WIN32
#include<windows.h>
#endif

LocalFileXmlInputSource::LocalFileXmlInputSource(const XMLCh *path, const XMLCh *base):
  xercesc::LocalFileInputSource(base,path)
{
  if (xercesc::XMLString::findAny(path, kPercent)!=0){
    XMLCh* e_path = ExpandEnvironment(path);
    this->setSystemId(e_path);
    delete e_path;
  }
}

LocalFileXmlInputSource::~LocalFileXmlInputSource()
{
}

xercesc::BinInputStream *LocalFileXmlInputSource::makeStream() const
{
  xercesc::BinFileInputStream *stream = new xercesc::BinFileInputStream(this->getSystemId());
  if (!stream->getIsOpen())
  {
    delete stream;
    return NULL;
  }
  return stream;
}

XmlInputSource *LocalFileXmlInputSource::createRelative(const XMLCh *relPath) const
{
  return new LocalFileXmlInputSource(relPath, this->getSystemId());
};

xercesc::InputSource *LocalFileXmlInputSource::getInputSource()
{
  return this;
}

XMLCh *LocalFileXmlInputSource::ExpandEnvironment(const XMLCh *path)
{
#ifdef _WIN32
  size_t i=ExpandEnvironmentStrings(path,NULL,0);
  XMLCh *temp = new XMLCh[i];
  ExpandEnvironmentStrings(path,temp,static_cast<DWORD>(i));
  return temp;
#else
  XMLSize_t i=xercesc::XMLString::stringLen(path);
  XMLCh *temp = new XMLCh[i];
  xercesc::XMLString::copyString(temp,path);
  return temp;
#endif
}