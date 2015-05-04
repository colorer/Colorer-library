#ifndef _COLORER_SHAREDXMLINPUTSOURCE_H_
#define _COLORER_SHAREDXMLINPUTSOURCE_H_

#include<common/Common.h>
#include<common/Logging.h>
#include <xercesc/sax/InputSource.hpp>
#include <xml/XmlInputSource.h>
#include <xercesc/util/BinFileInputStream.hpp>

class SharedXmlInputSource : XmlInputSource
{
public:

  static SharedXmlInputSource* getSharedInputSource(const XMLCh* path, const XMLCh* base);
  xercesc::InputSource* getInputSource();
  /** Increments reference counter */
  int addref();
  /** Decrements reference counter */
  int delref();

  void openStream();
  const XMLByte* getStream()
  {
    if (mSrc == null) {
      openStream();
    }
    return mSrc;
  }

  XMLSize_t length() const
  {
    return mSize;
  }

  static SharedXmlInputSource* getShared(String* name)
  {
    return isHash->find(name)->second;
  }

private:
  SharedXmlInputSource(XmlInputSource* source);
  ~SharedXmlInputSource();

  static std::unordered_map<SString, SharedXmlInputSource*>* isHash;

  XmlInputSource* is;
  int ref_count;
  XMLByte* mSrc;
  XMLSize_t mSize;
};

#endif