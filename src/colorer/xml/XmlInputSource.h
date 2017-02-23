#ifndef _COLORER_XMLINPUTSOURCE_H_
#define _COLORER_XMLINPUTSOURCE_H_

#include <vector>
#include <xercesc/sax/InputSource.hpp>
#include <colorer/Common.h>
#include <xercesc/util/XMLUniDefs.hpp>

const XMLCh kJar[] = {xercesc::chLatin_j, xercesc::chLatin_a, xercesc::chLatin_r, xercesc::chColon, xercesc::chNull};
const XMLCh kPercent[] = {xercesc::chPercent, xercesc::chNull};

class XmlInputSource;

typedef std::unique_ptr<XmlInputSource> uXmlInputSource;

/**
* @brief Class to creat xercesc::InputSource
*/
class XmlInputSource: public xercesc::InputSource
{
public:
  /**
  * @brief Statically creates instance of uXmlInputSource object,
  * possibly based on parent source stream.
  * @param base Base stream, used to resolve relative paths.
  * @param path Could be relative file location, absolute file
  */
  static uXmlInputSource newInstance(const XMLCh* path, XmlInputSource* base);

  /**
  * @brief Tries statically create instance of InputSource object,
  * according to passed path string.
  * @param path Could be relative file location, absolute file
  */
  static uXmlInputSource newInstance(const XMLCh* path, const XMLCh* base);

  /**
  * @brief Creates inherited InputSource with the same type
  * relatively to the current.
  * @param relPath Relative URI part.
  */
  virtual uXmlInputSource createRelative(const XMLCh* relPath) const = 0;

  virtual xercesc::InputSource* getInputSource()
  {
    return nullptr;
  }

  virtual ~XmlInputSource() {};

  static UString getAbsolutePath(const String* basePath, const String* relPath);
  static XMLCh* ExpandEnvironment(const XMLCh* path);
  static bool isRelative(const String* path);
  static UString getClearPath(const String* basePath, const String* relPath);
  static bool isDirectory(const String* path);
  static void getFileFromDir(const String* relPath, std::vector<SString>& files);
protected:
  XmlInputSource() {};

private:
  XmlInputSource(XmlInputSource const &) = delete;
  XmlInputSource &operator=(XmlInputSource const &) = delete;
  XmlInputSource(XmlInputSource &&) = delete;
  XmlInputSource &operator=(XmlInputSource &&) = delete;
};


#endif //_COLORER_XMLINPUTSOURCE_H_

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
