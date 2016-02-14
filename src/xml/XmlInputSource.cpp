#include <xml/XmlInputSource.h>
#include <xml/LocalFileXmlInputSource.h>
#include <xml/ZipXmlInputSource.h>
#include <common/Exception.h>
#include <xercesc/util/XMLString.hpp>
#ifdef __unix__
#include <dirent.h>
#include <sys/stat.h>
#endif
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#endif

uXmlInputSource XmlInputSource::newInstance(const XMLCh* path, XmlInputSource* base)
{
  if (xercesc::XMLString::startsWith(path, kJar)) {
    return std::make_unique<ZipXmlInputSource>(path, base);
  }
  if (base) {
    return base->createRelative(path);
  }
  return std::make_unique<LocalFileXmlInputSource>(path, nullptr);
}

uXmlInputSource XmlInputSource::newInstance(const XMLCh* path, const XMLCh* base)
{
  if (!path || (*path == '\0')) {
    throw InputSourceException("XmlInputSource::newInstance: path is nullptr");
  }
  if (xercesc::XMLString::startsWith(path, kJar) || (base != nullptr && xercesc::XMLString::startsWith(base, kJar))) {
    return std::make_unique<ZipXmlInputSource>(path, base);
  }
  return std::make_unique<LocalFileXmlInputSource>(path, base);
}

UString XmlInputSource::getAbsolutePath(const String* basePath, const String* relPath)
{
  int root_pos = basePath->lastIndexOf('/');
  int root_pos2 = basePath->lastIndexOf('\\');
  if (root_pos2 > root_pos) {
    root_pos = root_pos2;
  }
  if (root_pos == -1) {
    root_pos = 0;
  } else {
    root_pos++;
  }
  std::unique_ptr<StringBuffer> newPath(new StringBuffer());
  newPath->append(DString(basePath, 0, root_pos)).append(relPath);
  return std::move(newPath);
}

XMLCh* XmlInputSource::ExpandEnvironment(const XMLCh* path)
{
#ifdef _WIN32
  size_t i = ExpandEnvironmentStrings(path, nullptr, 0);
  XMLCh* temp = new XMLCh[i];
  ExpandEnvironmentStrings(path, temp, static_cast<DWORD>(i));
  return temp;
#else
  //TODO реализовать под nix
  XMLSize_t i = xercesc::XMLString::stringLen(path);
  XMLCh* temp = new XMLCh[i];
  xercesc::XMLString::copyString(temp, path);
  return temp;
#endif
}

bool XmlInputSource::isRelative(const String* path)
{
  if (path->indexOf(':') != -1 && path->indexOf(':') < 10) return false;
  if (path->indexOf('/') == 0 || path->indexOf('\\') == 0) return false;
  return true;
}

UString XmlInputSource::getClearPath(const String* basePath, const String* relPath)
{
  UString clear_path(new SString(relPath));
  if (relPath->indexOf(DString("%")) != -1) {
    XMLCh* e_path = ExpandEnvironment(clear_path.get()->getWChars());
    clear_path.reset(new SString(DString(e_path)));
    delete e_path;
  }
  if (isRelative(clear_path.get())) {
    clear_path = std::move(getAbsolutePath(basePath, clear_path.get()));
    if (clear_path->startsWith(DString("file://"))) {
      clear_path.reset(new SString(clear_path.get(), 7, -1));
    }
  }
  return std::move(clear_path);
}

bool XmlInputSource::isDirectory(const String* path)
{
  bool is_dir = false;
#ifdef _WIN32
  // stat on win_xp and vc2015 have bug.
  DWORD dwAttrs = GetFileAttributes(path->getWChars());
  if (dwAttrs == INVALID_FILE_ATTRIBUTES) {
    throw Exception(StringBuffer("Can't get info for file/path: ") + path);
  }
  else if (dwAttrs & FILE_ATTRIBUTE_DIRECTORY) {
    is_dir = true;
  }
#else

  struct stat st;
  int ret = stat(path->getChars(), &st);

  if (ret == -1) {
    throw Exception(StringBuffer("Can't get info for file/path: ") + path);
  }
  else if ((st.st_mode & S_IFDIR)) {
    is_dir = true;
  }
#endif

  return is_dir;
}

#ifdef _WIN32
void XmlInputSource::getFileFromDir(const String* relPath, std::vector<SString> &files)
{
  WIN32_FIND_DATA ffd;
  HANDLE dir = FindFirstFile((StringBuffer(relPath) + "\\*.*").getTChars(), &ffd);
  if (dir != INVALID_HANDLE_VALUE) {
    while (true) {
      if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        files.push_back(StringBuffer(relPath) + "\\" + SString(ffd.cFileName));
      }
      if (FindNextFile(dir, &ffd) == FALSE) {
        break;
      }
    }
    FindClose(dir);
  }
}
#endif

#ifdef __unix__
void XmlInputSource::getFileFromDir(const String* relPath, std::vector<SString> &files)
{
  DIR* dir = opendir(relPath->getChars());
  if (dir != nullptr) {
    dirent* dire;
    while ((dire = readdir(dir)) != nullptr) {
      struct stat st;
      stat((StringBuffer(relPath) + "/" + dire->d_name).getChars(), &st);
      if (!(st.st_mode & S_IFDIR)) {
        files.push_back(StringBuffer(relPath) + "/" + dire->d_name);
      }
    }
  }
}
#endif