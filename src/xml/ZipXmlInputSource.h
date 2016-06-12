#ifndef _COLORER_ZIPINPUTSOURCE_H_
#define _COLORER_ZIPINPUTSOURCE_H_

#include<unicode/String.h>
#include <xml/XmlInputSource.h>
#include <xercesc/util/BinFileInputStream.hpp>
#include <xml/SharedXmlInputSource.h>


class ZipXmlInputSource : public XmlInputSource, public xercesc::InputSource
{
public:
	ZipXmlInputSource(const XMLCh *path, const XMLCh *base);
  ZipXmlInputSource(const XMLCh *path, const XMLCh *base, bool faked);
  ZipXmlInputSource(const XMLCh *path, XmlInputSource *base);
  ~ZipXmlInputSource();
	xercesc::BinInputStream* makeStream() const;
  ZipXmlInputSource *createRelative(const XMLCh *relPath) const;
  xercesc::InputSource *getInputSource();
private:
  void create(const XMLCh *path, const XMLCh *base);
  String *inJarLocation;
  SharedXmlInputSource *jarIS;
};


class UnZip : public xercesc::BinInputStream
{
public:
  UnZip(const XMLByte* src, XMLSize_t size, const String *path);
  ~UnZip();
  XMLFilePos curPos() const;

  XMLSize_t readBytes(XMLByte* const toFill, const XMLSize_t maxToRead);

  virtual const XMLCh* getContentType() const;

private:

  const XMLByte* const    mSrc;
  const XMLSize_t         mSize;
  const String*           path;
  XMLSize_t               mPos;
  XMLSize_t               mBoundary;
  byte *stream;
  int len;
};

#endif