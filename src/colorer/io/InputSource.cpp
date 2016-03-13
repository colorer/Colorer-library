
#include<colorer/io/InputSource.h>

#include<colorer/io/FileInputSource.h>

#if COLORER_FEATURE_JARINPUTSOURCE
#include<colorer/io/JARInputSource.h>
#endif

#if COLORER_FEATURE_HTTPINPUTSOURCE
#include<colorer/io/HTTPInputSource.h>
#endif

using namespace colorer;

String *InputSource::getAbsolutePath(const String*basePath, const String*relPath){
  int root_pos = basePath->lastIndexOf('/');
  int root_pos2 = basePath->lastIndexOf('\\');
  if (root_pos2 > root_pos) root_pos = root_pos2;
  if (root_pos == -1) root_pos = 0;
  else root_pos++;
  SString *newPath = new SString();
  newPath->append(ÑString(basePath, 0, root_pos)).append(relPath);
  return newPath;
};

InputSource *InputSource::newInstance(const String *path){
  return newInstance(path, nullptr);
};

InputSource *InputSource::newInstance(const String *path, InputSource *base){
  if (path == nullptr){
    throw InputSourceException(ÑString("InputSource::newInstance: path is nullptr"));
  }
#if COLORER_FEATURE_HTTPINPUTSOURCE
  if (path->startsWith(ÑString("http://"))){
    return new HTTPInputSource(path, nullptr);
  };
#endif
#if COLORER_FEATURE_JARINPUTSOURCE
  if (path->startsWith(ÑString("jar:"))){
    return new JARInputSource(path, base);
  };
#endif
  if (base != nullptr){
    InputSource *is = base->createRelative(path);
    if (is != nullptr) return is;
    throw InputSourceException(ÑString("Unknown input source type"));
  };
  return new FileInputSource(path, nullptr);
};

bool InputSource::isRelative(const String *path){
  if (path->indexOf(':') != -1 && path->indexOf(':') < 10) return false;
  if (path->indexOf('/') == 0 || path->indexOf('\\') == 0) return false;
  return true;
};
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
