#include<common/Logging.h>
#include <xml/SharedXmlInputSource.h>
#include <xercesc/util/BinFileInputStream.hpp>

std::unordered_map<SString, SharedXmlInputSource*>* SharedXmlInputSource::isHash = null;

int SharedXmlInputSource::addref()
{
  return ++ref_count;
}

int SharedXmlInputSource::delref()
{
  if (ref_count == 0) {
    CLR_ERROR("SharedXmlInputSource", "delref: already zeroed references");
  }
  ref_count--;
  if (ref_count <= 0) {
    delete this;
    return -1;
  }
  return ref_count;
}

SharedXmlInputSource::SharedXmlInputSource(XmlInputSource* source)
{
  is = source;
  ref_count = 1;
  mSrc = null;
  mSize = 0;
}

SharedXmlInputSource::~SharedXmlInputSource()
{
  isHash->erase(&DString(is->getInputSource()->getSystemId()));
  if (isHash->size() == 0) {
    delete isHash;
    isHash = NULL;
    delete mSrc;
  }
  delete is;
}

SharedXmlInputSource* SharedXmlInputSource::getSharedInputSource(const XMLCh* path, const XMLCh* base)
{
  XmlInputSource* tempis = XmlInputSource::newInstance(path, base);

  if (isHash == null) {
    isHash = new std::unordered_map<SString, SharedXmlInputSource*>();
  }

  SharedXmlInputSource* sis = null;
  auto s = isHash->find(&DString(tempis->getInputSource()->getSystemId()));
  if (s != isHash->end()) {
    sis = s->second;
  }

  if (sis == null) {
    sis = new SharedXmlInputSource(tempis);
    std::pair<SString, SharedXmlInputSource*> pp(DString(tempis->getInputSource()->getSystemId()), sis);
    isHash->emplace(pp);
    return sis;
  } else {
    delete tempis;
  }

  sis->addref();
  return sis;
}

xercesc::InputSource* SharedXmlInputSource::getInputSource()
{
  return is->getInputSource();
}

void SharedXmlInputSource::openStream()
{
  if (mSrc) {
    return;
  }
  xercesc::BinFileInputStream* bfis;
  bfis = (xercesc::BinFileInputStream*)is->getInputSource()->makeStream();
  mSize = (XMLSize_t)bfis->getSize();
  mSrc = new XMLByte[mSize];
  bfis->readBytes(mSrc, mSize);
  delete bfis;
}
