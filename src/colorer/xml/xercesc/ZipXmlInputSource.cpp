#include "colorer/xml/xercesc/ZipXmlInputSource.h"
#include "colorer/Exception.h"
#include "colorer/utils/Environment.h"
#include "colorer/zip/MemoryFile.h"

ZipXmlInputSource::ZipXmlInputSource(const XMLCh* path, const XMLCh* base)
{
  create(path, base);
}

void ZipXmlInputSource::create(const XMLCh* path, const XMLCh* base)
{
  const auto kJar_len = xercesc::XMLString::stringLen(kJar);
  if (xercesc::XMLString::startsWith(path, kJar)) {
    int path_idx = xercesc::XMLString::lastIndexOf(path, '!');
    if (path_idx == -1) {
      throw InputSourceException("Bad jar uri format: " + UnicodeString(path));
    }

    auto bpath = std::make_unique<XMLCh[]>(path_idx - kJar_len + 1);
    xercesc::XMLString::subString(bpath.get(), path, kJar_len, path_idx);
    jar_input_source = SharedXmlInputSource::getSharedInputSource(bpath.get(), base);

    in_jar_location = UnicodeString(UnicodeString(path), path_idx + 1);
  }
  else if (base != nullptr && xercesc::XMLString::startsWith(base, kJar)) {
    int base_idx = xercesc::XMLString::lastIndexOf(base, '!');
    if (base_idx == -1) {
      throw InputSourceException("Bad jar uri format: " + UnicodeString(path));
    }

    auto bpath = std::make_unique<XMLCh[]>(base_idx - kJar_len + 1);
    xercesc::XMLString::subString(bpath.get(), base, kJar_len, base_idx);
    jar_input_source = SharedXmlInputSource::getSharedInputSource(bpath.get(), nullptr);

    auto in_base = std::make_unique<UnicodeString>(UnicodeString(base), base_idx + 1);
    in_jar_location = colorer::Environment::getAbsolutePath(*in_base.get(), UnicodeString(path));
  }
  else {
    throw InputSourceException("Can't create jar source");
  }

  UnicodeString str("jar:");
  str.append(UnicodeString(jar_input_source->getInputSource()->getSystemId()));
  str.append("!");
  str.append(in_jar_location);
  setSystemId(UStr::to_xmlch(&str).get());
  source_path = std::make_unique<UnicodeString>(str);
}

ZipXmlInputSource::~ZipXmlInputSource()
{
  jar_input_source->delref();
}

xercesc::InputSource* ZipXmlInputSource::getInputSource() const
{
  return (xercesc::InputSource*) this;
}

xercesc::BinInputStream* ZipXmlInputSource::makeStream() const
{
  return new UnZip(jar_input_source->getSrc(), jar_input_source->getSize(), &in_jar_location);
}

UnZip::UnZip(const XMLByte* src, XMLSize_t size, const UnicodeString* path) : mPos(0), stream(nullptr)
{
  stream = unzip(src, static_cast<int>(size), *path);
}

XMLFilePos UnZip::curPos() const
{
  return mPos;
}

XMLSize_t UnZip::readBytes(XMLByte* const toFill, const XMLSize_t maxToRead)
{
  XMLSize_t mBoundary = stream->size();
  XMLSize_t remain = mBoundary - mPos;
  XMLSize_t toRead = (maxToRead < remain) ? maxToRead : remain;
  memcpy(toFill, stream->data() + mPos, toRead);
  mPos += toRead;
  return toRead;
}

const XMLCh* UnZip::getContentType() const
{
  return nullptr;
}

UnZip::~UnZip() = default;
