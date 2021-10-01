#include <colorer/Exception.h>
#include <colorer/common/UStr.h>
#include <colorer/xml/LocalFileXmlInputSource.h>
#include <filesystem>
#include <xercesc/util/BinFileInputStream.hpp>

LocalFileXmlInputSource::LocalFileXmlInputSource(const XMLCh* path, const XMLCh* base)
{
  auto upath = UnicodeString(path);
  auto ubase = UnicodeString(base);
  auto clear_path = XmlInputSource::getClearFilePath(&ubase, &upath);

  if (std::filesystem::is_regular_file(UStr::to_stdwstr(clear_path))) {
    input_source = std::make_unique<xercesc::LocalFileInputSource>(UStr::to_xmlch(clear_path.get()).get());
    // file is not open yet, only after makeStream
  } else {
    throw InputSourceException(*clear_path + " isn't regular file.");
  }
}

xercesc::BinInputStream* LocalFileXmlInputSource::makeStream() const
{
  auto stream = std::make_unique<xercesc::BinFileInputStream>(input_source->getSystemId());
  if (!stream->getIsOpen()) {
    throw InputSourceException("Can't open file '" + UnicodeString(input_source->getSystemId()) + "'");
  }
  return stream.release();
}

xercesc::InputSource* LocalFileXmlInputSource::getInputSource()
{
  return input_source.get();
}
