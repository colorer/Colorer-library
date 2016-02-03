#ifndef _COLORER_SHAREDXMLINPUTSOURCE_H_
#define _COLORER_SHAREDXMLINPUTSOURCE_H_

#include <common/Common.h>
#include <xercesc/sax/InputSource.hpp>
#include <xml/XmlInputSource.h>

class SharedXmlInputSource
{
public:

  static SharedXmlInputSource* getSharedInputSource(const XMLCh* path, const XMLCh* base);
  xercesc::InputSource* getInputSource() const;

  /** Increments reference counter */
  int addref();
  /** Decrements reference counter */
  int delref();

  static SharedXmlInputSource* getShared(String* name)
  {
    return isHash->find(name)->second;
  }

  XMLSize_t getSize() const;
  XMLByte* getSrc() const;
private:
  SharedXmlInputSource(uXmlInputSource &source);
  ~SharedXmlInputSource();

  static std::unordered_map<SString, SharedXmlInputSource*>* isHash;

  uXmlInputSource is;
  int ref_count;
  std::unique_ptr<XMLByte[]> mSrc;
  XMLSize_t mSize;

  SharedXmlInputSource(SharedXmlInputSource const &) = delete;
  SharedXmlInputSource &operator=(SharedXmlInputSource const &) = delete;
  SharedXmlInputSource(SharedXmlInputSource &&) = delete;
  SharedXmlInputSource &operator=(SharedXmlInputSource &&) = delete;
};

inline XMLSize_t SharedXmlInputSource::getSize() const
{
  return mSize;
}

inline XMLByte* SharedXmlInputSource::getSrc() const
{
  return mSrc.get();
}

#endif //_COLORER_SHAREDXMLINPUTSOURCE_H_