#include <xml/SharedXmlInputSource.h>

Hashtable<SharedXmlInputSource*> *SharedXmlInputSource::isHash = null;

int SharedXmlInputSource::addref(){
  return ++ref_count;
}

int SharedXmlInputSource::delref(){
  if (ref_count == 0){
    CLR_ERROR("SharedXmlInputSource", "delref: already zeroed references");
  }
  ref_count--;
  if (ref_count <= 0){
    delete this;
    return -1;
  }
  return ref_count;
}

SharedXmlInputSource::SharedXmlInputSource(XmlInputSource *source){
  is = source;
  ref_count = 1;
  mSrc = null;
  mSize = 0;
  bfis = null;
}

SharedXmlInputSource::~SharedXmlInputSource(){
  isHash->remove(&DString(is->getInputSource()->getSystemId()));
  if (isHash->size()==0){ 
    delete isHash;
    isHash = NULL;
    delete mSrc;
    delete bfis;
  }
  delete is;
}

SharedXmlInputSource *SharedXmlInputSource::getSharedInputSource(const XMLCh *path, const XMLCh *base)
{
  XmlInputSource *tempis = XmlInputSource::newInstance(path, base);

  if (isHash == null){
    isHash = new Hashtable<SharedXmlInputSource*>();
  }

  SharedXmlInputSource *sis = isHash->get(&DString(tempis->getInputSource()->getSystemId()));

  if (sis == null){
    sis = new SharedXmlInputSource(tempis);
    isHash->put(&DString(tempis->getInputSource()->getSystemId()), sis);
    return sis;
  }else{
    delete tempis;
  }

  sis->addref();
  return sis;
}

xercesc::InputSource *SharedXmlInputSource::getInputSource()
{
  return is->getInputSource();
}

void SharedXmlInputSource::openStream()
{
  if (bfis) return;
  bfis = (xercesc::BinFileInputStream*)is->getInputSource()->makeStream();
  mSize = (XMLSize_t)bfis->getSize();
  mSrc = new XMLByte[mSize];
  bfis->readBytes(mSrc, mSize);
}
