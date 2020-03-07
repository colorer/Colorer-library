#include<cstdio>
#include<cstring>
#include<sys/stat.h>
#include<sys/timeb.h>
#include<fcntl.h>
#include<ctime>

#if defined _WIN32
#include<io.h>
#include<windows.h>
#endif
#if defined __unix__ || defined __GNUC__
#include<unistd.h>
#endif
#ifndef O_BINARY
#define O_BINARY 0x0
#endif

#include<colorer/io/FileInputSource.h>

FileInputSource::FileInputSource(const UnicodeString *basePath, FileInputSource *base){
  bool prefix = true;
  if (basePath->startsWith("file://")){
    baseLocation = new UnicodeString(*basePath, 7, -1);
  }else if (basePath->startsWith("file:/")){
    baseLocation = new UnicodeString(*basePath, 6, -1);
  }else if (basePath->startsWith("file:")){
    baseLocation = new UnicodeString(*basePath, 5, -1);
  }else{
    if (isRelative(basePath) && base != nullptr)
      baseLocation = getAbsolutePath(base->getLocation(), basePath);
    else
      baseLocation = new UnicodeString(*basePath);
    prefix = false;
  }
#if defined _WIN32
   // replace the environment variables to their values
  size_t i=ExpandEnvironmentStringsW(baseLocation->getWChars(),nullptr,0);
  wchar_t *temp = new wchar_t[i];
  ExpandEnvironmentStringsW(baseLocation->getWChars(),temp,static_cast<DWORD>(i));
  delete baseLocation;
  baseLocation = new SString(temp);
  delete[] temp;
#endif
  if(prefix && (baseLocation->indexOf(':') == -1 || baseLocation->indexOf(':') > 10) && !baseLocation->startsWith("/")){
    auto *n_baseLocation = new UnicodeString();
    n_baseLocation->append("/").append(*baseLocation);
    delete baseLocation;
    baseLocation = n_baseLocation;
  }
  stream = nullptr;
}

FileInputSource::~FileInputSource(){
  delete baseLocation;
  delete[] stream;
}
colorer::InputSource *FileInputSource::createRelative(const UnicodeString *relPath){
  return new FileInputSource(relPath, this);
}

const UnicodeString *FileInputSource::getLocation() const{
  return baseLocation;
}

const byte *FileInputSource::openStream()
{
  if (stream != nullptr) throw InputSourceException("openStream(): source stream already opened: '" + *baseLocation+"'");
#if defined _WIN32
  int source = _wopen(baseLocation->getWChars(), O_BINARY);
#else
  int source = open(UStr::to_stdstr(baseLocation).c_str(), O_BINARY);
#endif
  if (source == -1)
    throw InputSourceException("Can't open file '" + *baseLocation+"'");
  struct stat st{};
  fstat(source, &st);
  len = st.st_size;

  stream = new byte[len];
  memset(stream,0, sizeof(byte)*len);
  read(source, stream, len);
  close(source);
  return stream;
}

void FileInputSource::closeStream(){
  if (stream == nullptr) throw InputSourceException("closeStream(): source stream is not yet opened");
  delete[] stream;
  stream = nullptr;
}

int FileInputSource::length() const{
  if (stream == nullptr)
    throw InputSourceException("length(): stream is not yet opened");
  return len;
}


