#include <xml/ZipXmlInputSource.h>
#include <contrib/minizip/unzip.h>
#include <xercesc/util/XMLString.hpp>
#include <common/io/MemoryFile.h>
#include "XStr.h"

ZipXmlInputSource::ZipXmlInputSource(const XMLCh *path, const XMLCh *base)
{
  create(path,base);
}

ZipXmlInputSource::ZipXmlInputSource(const XMLCh *path, XmlInputSource *base)
{
  const XMLCh *base_path = nullptr;
  if (base)
  {
    base_path = base->getInputSource()->getSystemId();
  }
  create(path, base_path);
}

void ZipXmlInputSource::create(const XMLCh *path, const XMLCh *base)
{
  if (path==nullptr || *path == '\0')
    throw Exception(StringBuffer("Can't create jar source"));
  int path_idx = xercesc::XMLString::lastIndexOf(path,'!');
  if (path_idx == -1) throw Exception(StringBuffer("Bad jar uri format: ") + DString(path));
  inJarLocation.reset(new SString(DString(path), path_idx+1, -1));
  XMLCh *bpath=new XMLCh [path_idx-4+1];
  xercesc::XMLString::subString(bpath,path,4,path_idx);
  jarIS = SharedXmlInputSource::getSharedInputSource( bpath, base);
  delete[] bpath;
  StringBuffer str("jar:");
  str.append(DString(jarIS->getInputSource()->getSystemId()));
  str.append(DString("!"));
  str.append(inJarLocation.get());
  setSystemId(str.getWChars());
}

ZipXmlInputSource::ZipXmlInputSource(const XMLCh *path, const XMLCh *base, bool faked)
{
  int base_idx = xercesc::XMLString::lastIndexOf(base,'!');
  XMLCh *bpath= new XMLCh [base_idx-4+1];
  xercesc::XMLString::subString(bpath,base,4,base_idx);

  DString d_bpath = DString(bpath);
  jarIS = SharedXmlInputSource::getShared(&d_bpath);
  delete[] bpath;
  jarIS->addref();

  String * in_base = new SString(DString(base), base_idx+1, -1);
  DString d_path = DString(path);
  inJarLocation = XmlInputSource::getAbsolutePath(in_base, &d_path);
  delete in_base;
  StringBuffer str("jar:");
  str.append(DString(jarIS->getInputSource()->getSystemId()));
  str.append(DString("!"));
  str.append(inJarLocation.get());
  setSystemId(str.getWChars());
}

ZipXmlInputSource::~ZipXmlInputSource()
{
  jarIS->delref();
}

uXmlInputSource ZipXmlInputSource::createRelative(const XMLCh *relPath) const
{
  return std::make_unique<ZipXmlInputSource>(relPath, this->getSystemId(), true);
}

xercesc::InputSource *ZipXmlInputSource::getInputSource()
{
  return this;
}

xercesc::BinInputStream* ZipXmlInputSource::makeStream() const   
{
  return new UnZip(jarIS->getSrc(), jarIS->getSize(), inJarLocation.get());
}


UnZip::UnZip(const XMLByte* src, XMLSize_t size, const String* path)
  : path(path), mPos(0), mBoundary(0), stream(nullptr), len(0)
{
  MemoryFile* mf = new MemoryFile;
  mf->stream = src;
  mf->length = size;
  zlib_filefunc_def zlib_ff;
  fill_mem_filefunc(&zlib_ff, mf);

  unzFile fid = unzOpen2(nullptr, &zlib_ff);

  if (fid == 0) {
    delete mf;
    unzClose(fid);
    throw InputSourceException(StringBuffer("Can't locate file in JAR content: '") + path + "'");
  }
  int ret = unzLocateFile(fid, path->getChars(), 0);
  if (ret != UNZ_OK)  {
    delete mf;
    unzClose(fid);
    throw InputSourceException(StringBuffer("Can't locate file in JAR content: '") + path + "'");
  }
  unz_file_info file_info;
  ret = unzGetCurrentFileInfo(fid, &file_info, nullptr, 0, nullptr, 0, nullptr, 0);
  if (ret != UNZ_OK)  {
    delete mf;
    unzClose(fid);
    throw InputSourceException(StringBuffer("Can't retrieve current file in JAR content: '") + path + "'");
  }

  len = file_info.uncompressed_size;
  stream = new byte[len];
  ret = unzOpenCurrentFile(fid);
  if (ret != UNZ_OK)  {
    delete mf;
    unzClose(fid);
    throw InputSourceException(StringBuffer("Can't open current file in JAR content: '") + path + "'");
  }
  ret = unzReadCurrentFile(fid, stream, len);
  if (ret <= 0) {
    delete mf;
    unzClose(fid);
    throw InputSourceException(StringBuffer("Can't read current file in JAR content: '") + path + "' (" + SString(ret) + ")");
  }
  ret = unzCloseCurrentFile(fid);
  if (ret == UNZ_CRCERROR) {
    delete mf;
    unzClose(fid);
    throw InputSourceException(StringBuffer("Bad JAR file CRC"));
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
  memcpy(toFill, stream + mPos, toRead);
  mPos += toRead;
  return toRead;
}

const XMLCh* UnZip::getContentType() const
{
  return nullptr;
}

UnZip::~UnZip()
{
  delete[] stream;
}
