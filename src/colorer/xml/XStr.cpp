#include <colorer/xml/XStr.h>


XStr::XStr(const XMLCh* const toTranscode)
{
  _xmlch.reset(xercesc::XMLString::replicate(toTranscode));
  std::unique_ptr<char> tmp_str(xercesc::XMLString::transcode(toTranscode));
  _string = std::string(tmp_str.get());

}

XStr::XStr(const std::string &toTranscode)
{
  if (toTranscode.empty()) {
    _xmlch = nullptr;
  } else {
    _string = toTranscode;
    _xmlch.reset(xercesc::XMLString::transcode(toTranscode.c_str()));
  }
}

XStr::XStr(const std::string* toTranscode)
{
  if (toTranscode == nullptr || toTranscode->empty()) {
    _xmlch = nullptr;
  } else {
    _string = *toTranscode;
    _xmlch.reset(xercesc::XMLString::transcode(toTranscode->c_str()));
  }
}

XStr::~XStr()
{
}

std::ostream &operator<<(std::ostream &stream, const XStr &x)
{
  if (x._string.empty()) {
    stream << "\\0";
  } else {
    stream << x._string;
  }
  return stream;
}

const char* XStr::get_char() const
{
  if (_string.empty()) {
    return nullptr;
  } else {
    return _string.c_str();
  }
}

const std::string* XStr::get_stdstr() const
{
  return &_string;
}

const XMLCh* XStr::get_xmlchar() const
{
  return _xmlch.get();
}
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
