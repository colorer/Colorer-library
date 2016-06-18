#include <colorer/xml/XmlInputSource.h>
#include <colorer/xml/LocalFileXmlInputSource.h>
#include <colorer/xml/ZipXmlInputSource.h>
#include <colorer/Exception.h>
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
    throw InputSourceException(CString("XmlInputSource::newInstance: path is nullptr"));
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
  std::unique_ptr<SString> newPath(new SString());
  newPath->append(CString(basePath, 0, root_pos)).append(relPath);
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
  if (relPath->indexOf(CString("%")) != -1) {
    XMLCh* e_path = ExpandEnvironment(clear_path.get()->getWChars());
    clear_path.reset(new SString(CString(e_path)));
    delete e_path;
  }
  if (isRelative(clear_path.get())) {
    clear_path = std::move(getAbsolutePath(basePath, clear_path.get()));
    if (clear_path->startsWith(CString("file://"))) {
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
    throw Exception(SString("Can't get info for file/path: ") + path);
  }
  else if (dwAttrs & FILE_ATTRIBUTE_DIRECTORY) {
    is_dir = true;
  }
#else

  struct stat st;
  int ret = stat(path->getChars(), &st);

  if (ret == -1) {
    throw Exception(SString("Can't get info for file/path: ") + path);
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
  HANDLE dir = FindFirstFile((SString(relPath) + "\\*.*").getWChars(), &ffd);
  if (dir != INVALID_HANDLE_VALUE) {
    while (true) {
      if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        files.push_back(SString(relPath) + "\\" + SString(ffd.cFileName));
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
      stat((SString(relPath) + "/" + dire->d_name).getChars(), &st);
      if (!(st.st_mode & S_IFDIR)) {
        files.push_back(SString(relPath) + "/" + dire->d_name);
      }
    }
  }
}
#endif
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Colorer Library.
 *
 * The Initial Developer of the Original Code is
 * Cail Lomecb <irusskih at gmail dot com>.
 * Portions created by the Initial Developer are Copyright (C) 1999-2009
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s): see file CONTRIBUTORS
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
