
#include<colorer/io/JARInputSource.h>
#include<colorer/io/MemoryFile.h>
#include<contrib/minizip/unzip.h>

JARInputSource::JARInputSource(const String *basePath, InputSource *base){
  if (basePath == nullptr)
    throw InputSourceException("Can't create jar source");
  // absolute jar uri
  int ex_idx = basePath->lastIndexOf('!');
  if (ex_idx == -1) throw InputSourceException("Bad jar uri format: " + UStr::to_unistr(basePath));

  inJarLocation = new SString(basePath, ex_idx+1, -1);
  
  CString bpath = CString(basePath, 4, ex_idx-4);
  sharedIS = SharedInputSource::getInputSource(&bpath, base);

  SString str("jar:");
  str.append(sharedIS->getLocation());
  str.append(CString("!"));
  str.append(inJarLocation);
  baseLocation = new SString(&str);

  stream = nullptr;
  len = 0;
}

JARInputSource::~JARInputSource(){
  sharedIS->delref();
  delete baseLocation;
  delete inJarLocation;
  delete stream;
}

JARInputSource::JARInputSource(const String *basePath, JARInputSource *base, bool faked){
  // relative jar uri
  JARInputSource *parent = base;
  if (parent == nullptr) throw InputSourceException("Bad jar uri format: " + UStr::to_unistr(basePath));
  sharedIS = parent->getShared();
  sharedIS->addref();

  inJarLocation = getAbsolutePath(parent->getInJarLocation(), basePath);

  SString str("jar:");
  str.append(sharedIS->getLocation());
  str.append(CString("!"));
  str.append(inJarLocation);
  baseLocation = new SString(&str);
  stream = nullptr;
  len = 0;
}

colorer::InputSource *JARInputSource::createRelative(const String *relPath){
  return new JARInputSource(relPath, this, true);
}

const String *JARInputSource::getLocation() const{
  return baseLocation;
}

const byte *JARInputSource::openStream()
{
  if (stream != nullptr)
    throw InputSourceException("openStream(): source stream already opened: '"+ UStr::to_unistr(baseLocation)+"'");

  MemoryFile *mf = new MemoryFile;
  mf->stream = sharedIS->getStream();
  mf->length = sharedIS->length();
  zlib_filefunc_def zlib_ff;
  fill_mem_filefunc(&zlib_ff, mf);

  unzFile fid = unzOpen2(nullptr, &zlib_ff);

  if (fid == 0) {
	  delete mf;
	  unzClose(fid);
	  throw InputSourceException("Can't locate file in JAR content: '"+ UStr::to_unistr(inJarLocation)+"'");
  }
  int ret = unzLocateFile(fid, inJarLocation->getChars(), 0);
  if (ret != UNZ_OK)  {
	  delete mf;
	  unzClose(fid);
	  throw InputSourceException("Can't locate file in JAR content: '"+ UStr::to_unistr(inJarLocation)+"'");
  }
  unz_file_info file_info;
  ret = unzGetCurrentFileInfo(fid, &file_info, nullptr, 0, nullptr, 0, nullptr, 0);
  if (ret != UNZ_OK)  {
	  delete mf;
	  unzClose(fid);
	  throw InputSourceException("Can't retrieve current file in JAR content: '"+ UStr::to_unistr(inJarLocation)+"'");
  }

  len = file_info.uncompressed_size;
  stream = new byte[len];
  ret = unzOpenCurrentFile(fid);
  if (ret != UNZ_OK)  {
	  delete mf;
	  unzClose(fid);
	  throw InputSourceException("Can't open current file in JAR content: '"+ UStr::to_unistr(inJarLocation)+"'");
  }
  ret = unzReadCurrentFile(fid, stream, len);
  if (ret <= 0) {
	  delete mf;
	  unzClose(fid);
	  throw InputSourceException("Can't read current file in JAR content: '"+ UStr::to_unistr(inJarLocation)+"' ("+ UStr::to_unistr(ret)+")");
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


