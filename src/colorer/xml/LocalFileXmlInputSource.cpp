#include <colorer/Exception.h>
#include <colorer/common/UStr.h>
#include <colorer/utils/Environment.h>
#include <colorer/xml/LocalFileXmlInputSource.h>
#include <xercesc/util/BinFileInputStream.hpp>

LocalFileXmlInputSource::LocalFileXmlInputSource(const XMLCh* path, const XMLCh* base)
{
  auto upath = UnicodeString(path);
  auto clear_path = Environment::expandEnvironment(&upath);
  input_source = std::make_unique<xercesc::LocalFileInputSource>(base, UStr::to_xmlch(clear_path.get()).get());
}

xercesc::BinInputStream* LocalFileXmlInputSource::makeStream() const
{
  auto stream = std::make_unique<xercesc::BinFileInputStream>(input_source->getSystemId());
  if (!stream->getIsOpen()) {
    throw InputSourceException("Can't open file '" + UnicodeString(input_source->getSystemId()) + "'");
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
