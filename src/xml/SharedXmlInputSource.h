#ifndef _COLORER_SHAREDXMLINPUTSOURCE_H_
#define _COLORER_SHAREDXMLINPUTSOURCE_H_

#include<common/Common.h>
#include<common/Hashtable.h>
#include<common/Logging.h>
#include <xercesc/sax/InputSource.hpp>
#include <xml/XmlInputSource.h>
#include <xercesc/util/BinFileInputStream.hpp>

class SharedXmlInputSource : XmlInputSource
{
public:

  static SharedXmlInputSource *getSharedInputSource(const XMLCh *path, const XMLCh *base);
  xercesc::InputSource *getInputSource();
  /** Increments reference counter */
  int addref();
  /** Decrements reference counter */
  int delref();

  void openStream();
  const XMLByte *getStream(){
    if (mSrc == null){
      openStream();
    }
    return mSrc;
  }
  XMLSize_t length() const{ return mSize; }

  static SharedXmlInputSource* getShared(String * name){
    return isHash->get(name); 
  };

private:
  SharedXmlInputSource(XmlInputSource *source);
  ~SharedXmlInputSource();

  static Hashtable<SharedXmlInputSource*> *isHash;

  XmlInputSource *is;
  int ref_count;
  XMLByte* mSrc;
	XMLSize_t mSize;
  xercesc::BinFileInputStream* bfis;
};

#endif