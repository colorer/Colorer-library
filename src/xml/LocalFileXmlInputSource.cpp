#include <xml/LocalFileXmlInputSource.h>
#include <xercesc/util/BinFileInputStream.hpp>

LocalFileXmlInputSource::LocalFileXmlInputSource(const XMLCh *path, const XMLCh *base):
  xercesc::LocalFileInputSource(base,path)
{
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
