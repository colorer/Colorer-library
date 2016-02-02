#include <xml/LocalFileXmlInputSource.h>
#include <xercesc/util/BinFileInputStream.hpp>
#include <xercesc/util/XMLString.hpp>
#include "XStr.h"
#ifdef _WIN32
#include<windows.h>
#endif

LocalFileXmlInputSource::LocalFileXmlInputSource(const XMLCh* path, const XMLCh* base)
{
  input_source.reset(new xercesc::LocalFileInputSource(base, path));
  if (xercesc::XMLString::findAny(path, kPercent) != nullptr) {
    XMLCh* e_path = ExpandEnvironment(path);
    input_source->setSystemId(e_path);
    delete e_path;
  }
}

LocalFileXmlInputSource::~LocalFileXmlInputSource()
{
}

xercesc::BinInputStream* LocalFileXmlInputSource::makeStream() const
{
  std::unique_ptr<xercesc::BinFileInputStream> stream (new xercesc::BinFileInputStream(input_source->getSystemId()));
  if (!stream->getIsOpen()) {
    throw InputSourceException("Can't open file '" + *XStr(input_source->getSystemId()).get_stdstr() + "'");
  }
  return stream.release();
}

uXmlInputSource LocalFileXmlInputSource::createRelative(const XMLCh* relPath) const
{
  return std::make_unique<LocalFileXmlInputSource>(relPath, input_source->getSystemId());
}

xercesc::InputSource* LocalFileXmlInputSource::getInputSource()
{
  return input_source.get();
}

XMLCh* LocalFileXmlInputSource::ExpandEnvironment(const XMLCh* path)
{
#ifdef _WIN32
  size_t i = ExpandEnvironmentStrings(path, nullptr, 0);
  XMLCh* temp = new XMLCh[i];
  ExpandEnvironmentStrings(path, temp, static_cast<DWORD>(i));
  return temp;
#else
  //TODO реализовать под nix
  XMLSize_t i = xercesc::XMLString::stringLen(path);
  XMLCh* temp = new XMLCh[i];
  xercesc::XMLString::copyString(temp, path);
  return temp;
#endif
}
