#ifndef _COLORER_ZIPINPUTSOURCE_H_
#define _COLORER_ZIPINPUTSOURCE_H_

#include <colorer/xml/SharedXmlInputSource.h>
#include <colorer/xml/XmlInputSource.h>
#include <xercesc/util/BinFileInputStream.hpp>

class ZipXmlInputSource : public XmlInputSource
{
 public:
  ZipXmlInputSource(const XMLCh* path, const XMLCh* base);
  ZipXmlInputSource(const XMLCh* path, XmlInputSource* base);
  ~ZipXmlInputSource() override;
  [[nodiscard]] xercesc::BinInputStream* makeStream() const override;
  uXmlInputSource createRelative(const XMLCh* relPath) const override;
  xercesc::InputSource* getInputSource() override;

  ZipXmlInputSource(ZipXmlInputSource const&) = delete;
  ZipXmlInputSource& operator=(ZipXmlInputSource const&) = delete;
  ZipXmlInputSource(ZipXmlInputSource&&) = delete;
  ZipXmlInputSource& operator=(ZipXmlInputSource&&) = delete;

 private:
  void create(const XMLCh* path, const XMLCh* base);
  uUnicodeString in_jar_location;
  SharedXmlInputSource* jar_input_source;
};

class UnZip : public xercesc::BinInputStream
{
 public:
  UnZip(const XMLByte* src, XMLSize_t size, const UnicodeString* path);
  ~UnZip() override;

  [[nodiscard]] XMLFilePos curPos() const override;
  XMLSize_t readBytes(XMLByte* const toFill, const XMLSize_t maxToRead) override;
  [[nodiscard]] const XMLCh* getContentType() const override;

  UnZip(UnZip const&) = delete;
  UnZip& operator=(UnZip const&) = delete;
  UnZip(UnZip&&) = delete;
  UnZip& operator=(UnZip&&) = delete;

 private:
  XMLSize_t mPos;
  XMLSize_t mBoundary;
  std::unique_ptr<byte[]> stream;
  int len;
};

#endif  //_COLORER_ZIPINPUTSOURCE_H_
