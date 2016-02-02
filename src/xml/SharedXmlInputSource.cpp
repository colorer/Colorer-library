#include <xml/SharedXmlInputSource.h>
#include <xercesc/util/BinFileInputStream.hpp>

std::unordered_map<SString, SharedXmlInputSource*>* SharedXmlInputSource::isHash = nullptr;

int SharedXmlInputSource::addref()
{
  return ++ref_count;
}

int SharedXmlInputSource::delref()
{
  ref_count--;
  if (ref_count <= 0) {
    delete this;
    return -1;
  }
  return ref_count;
}

SharedXmlInputSource::SharedXmlInputSource(uXmlInputSource &source)
{
  ref_count = 1;
  is = std::move(source);
  std::unique_ptr<xercesc::BinFileInputStream> bfis(static_cast<xercesc::BinFileInputStream*>(is->getInputSource()->makeStream()));
  mSize = static_cast<XMLSize_t>(bfis->getSize());
  mSrc.reset(new XMLByte[mSize]);
  bfis->readBytes(mSrc.get(), mSize);
}

SharedXmlInputSource::~SharedXmlInputSource()
{
  DString d_id = DString(is->getInputSource()->getSystemId());
  isHash->erase(&d_id);
  if (isHash->size() == 0) {
    delete isHash;
    isHash = nullptr;
  }
}

SharedXmlInputSource* SharedXmlInputSource::getSharedInputSource(const XMLCh* path, const XMLCh* base)
{
  uXmlInputSource tempis = XmlInputSource::newInstance(path, base);

  if (isHash == nullptr) {
    isHash = new std::unordered_map<SString, SharedXmlInputSource*>();
  }

  SharedXmlInputSource* sis = nullptr;
  DString d_id = DString(tempis->getInputSource()->getSystemId());
  auto s = isHash->find(&d_id);
  if (s != isHash->end()) {
    sis = s->second;
  }

  if (sis == nullptr) {
    sis = new SharedXmlInputSource(tempis);
    std::pair<SString, SharedXmlInputSource*> pp(DString(sis->getInputSource()->getSystemId()), sis);
    isHash->emplace(pp);
    return sis;
  }
  sis->addref();
  return sis;
}

xercesc::InputSource* SharedXmlInputSource::getInputSource() const
{
  return is->getInputSource();
}
