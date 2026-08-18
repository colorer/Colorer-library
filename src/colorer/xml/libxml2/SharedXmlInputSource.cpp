#include "colorer/xml/libxml2/SharedXmlInputSource.h"
#include <fstream>
#include "colorer/Exception.h"
#include "colorer/utils/Environment.h"

int SharedXmlInputSource::addref()
{
  return ++ref_count;
}

int SharedXmlInputSource::delref()
{
  ref_count--;
  if (ref_count <= 0) {
    delete this;
    return -1;
  }
  return ref_count;
}

SharedXmlInputSource* SharedXmlInputSource::getSharedInputSource(const UnicodeString& path)
{
  return new SharedXmlInputSource(path);
}

SharedXmlInputSource::SharedXmlInputSource(const UnicodeString& path) : source_path(path) {}

SharedXmlInputSource::~SharedXmlInputSource() = default;

int SharedXmlInputSource::getSize() const
{
  return mSize;
}

byte* SharedXmlInputSource::getSrc() const
{
  return mSrc.get();
}

void SharedXmlInputSource::open()
{
  if (!is_open) {
    std::ifstream f(colorer::Environment::to_filepath(&source_path), std::ios::in | std::ios::binary);
    if (!f.is_open()) {
      COLORER_LOG_ERROR("failed to open %", source_path);
      throw InputSourceException("failed to open " + source_path);
    }
    mSize = static_cast<int>(colorer::Environment::getFileSize(source_path));
    mSrc.reset(new byte[mSize]);
    f.read(reinterpret_cast<std::istream::char_type*>(mSrc.get()), mSize);
    f.close();
    is_open = true;
  }
}
