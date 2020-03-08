#include <colorer/Exception.h>
#include <colorer/xml/LocalFileXmlInputSource.h>
#include <colorer/xml/XmlInputSource.h>
#include <colorer/xml/ZipXmlInputSource.h>
#include <xercesc/util/XMLString.hpp>
#include <memory>
#ifdef __unix__
#include <dirent.h>
#include <sys/stat.h>
#endif
#ifdef WIN32
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

uUnicodeString XmlInputSource::getAbsolutePath(const UnicodeString* basePath, const UnicodeString* relPath)
{
  auto root_pos = basePath->lastIndexOf('/');
  auto root_pos2 = basePath->lastIndexOf('\\');
  if (root_pos2 > root_pos) {
    root_pos = root_pos2;
  }
  if (root_pos == -1) {
    root_pos = 0;
  } else {
    root_pos++;
  }
  std::unique_ptr<UnicodeString> newPath(new UnicodeString());
  newPath->append(UnicodeString(*basePath, 0, root_pos)).append(*relPath);
  return newPath;
}

XMLCh* XmlInputSource::ExpandEnvironment(const XMLCh* path)
{
#ifdef WIN32
  size_t i = ExpandEnvironmentStringsW((wchar_t*)path, nullptr, 0);
  auto* temp = new XMLCh[i];
  ExpandEnvironmentStringsW((wchar_t*)path, (wchar_t*)temp, static_cast<DWORD>(i));
  return temp;
#else
  // TODO реализовать под nix
  XMLSize_t i = xercesc::XMLString::stringLen(path);
  XMLCh* temp = new XMLCh[i];
  xercesc::XMLString::copyString(temp, path);
  return temp;
#endif
}

bool XmlInputSource::isRelative(const UnicodeString* path)
{
  if (path->indexOf(':') != -1 && path->indexOf(':') < 10)
    return false;
  if (path->indexOf('/') == 0 || path->indexOf('\\') == 0)
    return false;
  return true;
}

uUnicodeString XmlInputSource::getClearPath(const UnicodeString* basePath, const UnicodeString* relPath)
{
  uUnicodeString clear_path(new UnicodeString(*relPath));
  if (relPath->indexOf("%") != -1) {
    XMLCh* e_path = ExpandEnvironment(UStr::to_xmlch(clear_path.get()).get());
    clear_path = std::make_unique<UnicodeString>(e_path);
    delete[] e_path;
  }
  if (isRelative(clear_path.get())) {
    clear_path = getAbsolutePath(basePath, clear_path.get());
    if (clear_path->startsWith("file://")) {
      clear_path = std::make_unique<UnicodeString>(*clear_path.get(), 7);
    }
  }
  return clear_path;
}

bool XmlInputSource::isDirectory(const UnicodeString* path)
{
  bool is_dir = false;
#ifdef WIN32
  // stat on win_xp and vc2015 have bug.
  DWORD dwAttrs = GetFileAttributes(UStr::to_stdstr(path).c_str());
  if (dwAttrs == INVALID_FILE_ATTRIBUTES) {
    throw Exception("Can't get info for file/path: " + *path);
  } else if (dwAttrs & FILE_ATTRIBUTE_DIRECTORY) {
    is_dir = true;
  }
#else

  struct stat st;
  int ret = stat(UStr::to_stdstr(path).c_str(), &st);

  if (ret == -1) {
    throw Exception("Can't get info for file/path: " + *path);
  } else if ((st.st_mode & S_IFDIR)) {
    is_dir = true;
  }
#endif

  return is_dir;
}

#ifdef WIN32
void XmlInputSource::getFileFromDir(const UnicodeString* relPath, std::vector<UnicodeString>& files)
{
  WIN32_FIND_DATA ffd;
  UnicodeString s= UnicodeString(UnicodeString(*relPath) + UnicodeString("\\*.*"));
  HANDLE dir = FindFirstFile(UStr::to_stdstr(&s).c_str(), &ffd);
  if (dir != INVALID_HANDLE_VALUE) {
    while (true) {
      if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        files.push_back(UnicodeString(*relPath) + "\\" + UnicodeString(ffd.cFileName));
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
void XmlInputSource::getFileFromDir(const UnicodeString* relPath, std::vector<UnicodeString>& files)
{
  DIR* dir = opendir(UStr::to_stdstr(relPath).c_str());
  if (dir != nullptr) {
    dirent* dire;
    while ((dire = readdir(dir)) != nullptr) {
      struct stat st;
      stat(UStr::to_stdstr(&UnicodeString(*relPath + "/" + dire->d_name)).c_str(), &st);
      if (!(st.st_mode & S_IFDIR)) {
        files.push_back(UnicodeString(*relPath) + "/" + dire->d_name);
      }
    }
  }
}
#endif
