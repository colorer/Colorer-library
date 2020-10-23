#include <colorer/common/UStr.h>
#include<colorer/io/JARInputSource.h>
#include<colorer/io/MemoryFile.h>
#ifdef ZLIB_SHORT_PATH
#include <unzip.h>
#else
#include <minizip/unzip.h>
#endif

JARInputSource::JARInputSource(const UnicodeString *basePath, InputSource *base){
  if (basePath == nullptr)
    throw InputSourceException("Can't create jar source");
  // absolute jar uri
  int ex_idx = basePath->lastIndexOf('!');
  if (ex_idx == -1) throw InputSourceException("Bad jar uri format: " + *basePath);

  inJarLocation = new UnicodeString(*basePath, ex_idx+1);

  UnicodeString bpath = UnicodeString(*basePath, 4, ex_idx-4);
  sharedIS = SharedInputSource::getInputSource(&bpath, base);

  UnicodeString str("jar:");
  str.append(*sharedIS->getLocation());
  str.append("!");
  str.append(*inJarLocation);
  baseLocation = new UnicodeString(str);

  stream = nullptr;
  len = 0;
}

JARInputSource::~JARInputSource(){
  sharedIS->delref();
  delete baseLocation;
  delete inJarLocation;
  delete stream;
}

JARInputSource::JARInputSource(const UnicodeString *basePath, JARInputSource *base, bool /*faked*/){
  // relative jar uri
  JARInputSource *parent = base;
  if (parent == nullptr) throw InputSourceException("Bad jar uri format: " + *basePath);
  sharedIS = parent->getShared();
  sharedIS->addref();

  inJarLocation = getAbsolutePath(parent->getInJarLocation(), basePath);

  UnicodeString str("jar:");
  str.append(*sharedIS->getLocation());
  str.append("!");
  str.append(*inJarLocation);
  baseLocation = new UnicodeString(str);
  stream = nullptr;
  len = 0;
}

colorer::InputSource *JARInputSource::createRelative(const UnicodeString *relPath){
  return new JARInputSource(relPath, this, true);
}

const UnicodeString *JARInputSource::getLocation() const{
  return baseLocation;
}

const byte *JARInputSource::openStream()
{
  if (stream != nullptr)
    throw InputSourceException("openStream(): source stream already opened: '"+ *baseLocation+"'");

  auto *mf = new MemoryFile;
  mf->stream = sharedIS->getStream();
  mf->length = sharedIS->length();
  zlib_filefunc_def zlib_ff;
  fill_mem_filefunc(&zlib_ff, mf);

  unzFile fid = unzOpen2(nullptr, &zlib_ff);

  if (fid == nullptr) {
	  delete mf;
	  unzClose(fid);
	  throw InputSourceException("Can't locate file in JAR content: '"+ *inJarLocation+"'");
  }
  int ret = unzLocateFile(fid, UStr::to_stdstr(inJarLocation).c_str(), 0);
  if (ret != UNZ_OK)  {
	  delete mf;
	  unzClose(fid);
	  throw InputSourceException("Can't locate file in JAR content: '"+ *inJarLocation+"'");
  }
  unz_file_info file_info;
  ret = unzGetCurrentFileInfo(fid, &file_info, nullptr, 0, nullptr, 0, nullptr, 0);
  if (ret != UNZ_OK)  {
	  delete mf;
	  unzClose(fid);
	  throw InputSourceException("Can't retrieve current file in JAR content: '"+ *inJarLocation+"'");
  }

  len = file_info.uncompressed_size;
  stream = new byte[len];
  ret = unzOpenCurrentFile(fid);
  if (ret != UNZ_OK)  {
	  delete mf;
	  unzClose(fid);
	  throw InputSourceException("Can't open current file in JAR content: '"+ *inJarLocation+"'");
  }
  ret = unzReadCurrentFile(fid, stream, len);
  if (ret <= 0) {
	  delete mf;
	  unzClose(fid);
	  throw InputSourceException("Can't read current file in JAR content: '"+ *inJarLocation+"' ("+ UStr::to_unistr(ret)+")");
  }
  ret = unzCloseCurrentFile(fid);
  if (ret == UNZ_CRCERROR) {
	  delete mf;
	  unzClose(fid);
	  throw InputSourceException("Bad JAR file CRC");
  }
  ret = unzClose(fid);
  delete mf;
  return stream;
}

void JARInputSource::closeStream(){
  if (stream == nullptr)
    throw InputSourceException("closeStream(): source stream is not yet opened");
  delete stream;
  stream = nullptr;
}

int JARInputSource::length() const{
  if (stream == nullptr)
    throw InputSourceException("length(): stream is not yet opened");
  return len;
}


