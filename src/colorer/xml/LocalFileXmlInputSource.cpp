#include <colorer/Exception.h>
#include <colorer/xml/LocalFileXmlInputSource.h>
#include <xercesc/util/BinFileInputStream.hpp>
#include <xercesc/util/XMLString.hpp>

LocalFileXmlInputSource::LocalFileXmlInputSource(const XMLCh* path, const XMLCh* base)
{
  input_source = std::make_unique<xercesc::LocalFileInputSource>(base, path);
  if (xercesc::XMLString::findAny(path, kPercent) != nullptr) {
    XMLCh* e_path = ExpandEnvironment(path);
    input_source->setSystemId(e_path);
    delete e_path;
  }
}

LocalFileXmlInputSource::~LocalFileXmlInputSource() = default;

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
