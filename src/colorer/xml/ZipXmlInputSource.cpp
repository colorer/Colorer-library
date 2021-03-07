#include <colorer/common/UStr.h>
#include <colorer/io/MemoryFile.h>
#include <colorer/xml/ZipXmlInputSource.h>
#include <minizip/unzip.h>
#include <xercesc/util/XMLString.hpp>

ZipXmlInputSource::ZipXmlInputSource(const XMLCh* path, const XMLCh* base)
{
  create(path, base);
}

ZipXmlInputSource::ZipXmlInputSource(const XMLCh* path, XmlInputSource* base)
{
  const XMLCh* base_path = nullptr;
  if (base) {
    base_path = base->getInputSource()->getSystemId();
  }
  create(path, base_path);
}

void ZipXmlInputSource::create(const XMLCh* path, const XMLCh* base)
{
  if (!path || *path == '\0') {
    throw InputSourceException("Can't create jar source");
  }
  if (xercesc::XMLString::startsWith(path, kJar)) {
    int path_idx = xercesc::XMLString::lastIndexOf(path, '!');
    if (path_idx == -1) {
      throw InputSourceException("Bad jar uri format: " + UnicodeString(path));
    }

    std::unique_ptr<XMLCh[]> bpath(new XMLCh[path_idx - 4 + 1]);
    xercesc::XMLString::subString(bpath.get(), path, 4, path_idx);
    jar_input_source = SharedXmlInputSource::getSharedInputSource(bpath.get(), base);

    in_jar_location = std::make_unique<UnicodeString>(UnicodeString(path), path_idx + 1);

  } else if (base != nullptr && xercesc::XMLString::startsWith(base, kJar)) {
    int base_idx = xercesc::XMLString::lastIndexOf(base, '!');
    if (base_idx == -1) {
      throw InputSourceException("Bad jar uri format: " + UnicodeString(path));
    }

    std::unique_ptr<XMLCh[]> bpath(new XMLCh[base_idx - 4 + 1]);
    xercesc::XMLString::subString(bpath.get(), base, 4, base_idx);
    jar_input_source = SharedXmlInputSource::getSharedInputSource(bpath.get(), nullptr);

    uUnicodeString in_base(new UnicodeString(UnicodeString(base), base_idx + 1));
    UnicodeString d_path = UnicodeString(path);
    in_jar_location = XmlInputSource::getAbsolutePath(in_base.get(), &d_path);

  } else {
    throw InputSourceException("Can't create jar source");
  }

  UnicodeString str("jar:");
  str.append(UnicodeString(jar_input_source->getInputSource()->getSystemId()));
  str.append("!");
  str.append(*in_jar_location.get());
  setSystemId(UStr::to_xmlch(&str).get());
}

ZipXmlInputSource::~ZipXmlInputSource()
{
  jar_input_source->delref();
}

uXmlInputSource ZipXmlInputSource::createRelative(const XMLCh* relPath) const
{
  return std::make_unique<ZipXmlInputSource>(relPath, this->getSystemId());
}

xercesc::InputSource* ZipXmlInputSource::getInputSource()
{
  return this;
}

xercesc::BinInputStream* ZipXmlInputSource::makeStream() const
{
  return new UnZip(jar_input_source->getSrc(), jar_input_source->getSize(), in_jar_location.get());
}

UnZip::UnZip(const XMLByte* src, XMLSize_t size, const UnicodeString* path) : mPos(0), mBoundary(0), stream(nullptr), len(0)
{
  auto* mf = new MemoryFile;
  mf->stream = src;
  mf->length = (int) size;
  zlib_filefunc_def zlib_ff;
  fill_mem_filefunc(&zlib_ff, mf);

  unzFile fid = unzOpen2(nullptr, &zlib_ff);

  if (!fid) {
    delete mf;
    unzClose(fid);
    throw InputSourceException("Can't locate file in JAR content: '" + *path + "'");
  }
  int ret = unzLocateFile(fid, UStr::to_stdstr(path).c_str(), 0);
  if (ret != UNZ_OK) {
    delete mf;
    unzClose(fid);
    throw InputSourceException("Can't locate file in JAR content: '" + *path + "'");
  }
  unz_file_info file_info;
  ret = unzGetCurrentFileInfo(fid, &file_info, nullptr, 0, nullptr, 0, nullptr, 0);
  if (ret != UNZ_OK) {
    delete mf;
    unzClose(fid);
    throw InputSourceException("Can't retrieve current file in JAR content: '" + *path + "'");
  }

  len = file_info.uncompressed_size;
  stream.reset(new byte[len]);
  ret = unzOpenCurrentFile(fid);
  if (ret != UNZ_OK) {
    delete mf;
    unzClose(fid);
    throw InputSourceException("Can't open current file in JAR content: '" + *path + "'");
  }
  ret = unzReadCurrentFile(fid, stream.get(), len);
  if (ret <= 0) {
    delete mf;
    unzClose(fid);
    throw InputSourceException("Can't read current file in JAR content: '" + *path + "' (" + ret + ")");
  }
  ret = unzCloseCurrentFile(fid);
  if (ret == UNZ_CRCERROR) {
    delete mf;
    unzClose(fid);
    throw InputSourceException("Bad JAR file CRC");
  }
  ret = unzClose(fid);
  delete mf;
}

XMLFilePos UnZip::curPos() const
{
  return mPos;
}

XMLSize_t UnZip::readBytes(XMLByte* const toFill, const XMLSize_t maxToRead)
{
  mBoundary = len;
  XMLSize_t remain = mBoundary - mPos;
  XMLSize_t toRead = (maxToRead < remain) ? maxToRead : remain;
  memcpy(toFill, stream.get() + mPos, toRead);
  mPos += toRead;
  return toRead;
}

const XMLCh* UnZip::getContentType() const
{
  return nullptr;
}

UnZip::~UnZip() = default;
