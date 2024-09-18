#include "colorer/xml/libxml2/SharedXmlInputSource.h"
#include <fstream>
#include "colorer/Common.h"
#ifdef COLORER_FEATURE_OLD_COMPILERS
#include "colorer/platform/filesystem.hpp"
namespace fs = ghc::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

std::unordered_map<UnicodeString, SharedXmlInputSource*>* SharedXmlInputSource::isHash = nullptr;

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
  if (isHash == nullptr) {
    isHash = new std::unordered_map<UnicodeString, SharedXmlInputSource*>();
  }

  const auto s = isHash->find(path);
  if (s != isHash->end()) {
    SharedXmlInputSource* sis = s->second;
    sis->addref();
    return sis;
  }

  auto* sis = new SharedXmlInputSource(path);
  isHash->emplace(path, sis);
  return sis;
}

SharedXmlInputSource::SharedXmlInputSource(const UnicodeString& path)
{
  source_path = path;
  is_open = false;
}

SharedXmlInputSource::~SharedXmlInputSource()
{
  // не нужно удалять объект, удаляемый из массива. мы и так уже в деструкторе
  isHash->erase(source_path);
  if (isHash->empty()) {
    delete isHash;
    isHash = nullptr;
  }
}

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
    std::ifstream f(UStr::to_stdstr(&source_path), std::ios::in | std::ios::binary);
    mSize = static_cast<int>(fs::file_size(UStr::to_stdstr(&source_path)));
    mSrc.reset(new byte[mSize]);
    f.read(reinterpret_cast<std::istream::char_type*>(mSrc.get()), mSize);
    f.close();
    is_open = true;
  }
}