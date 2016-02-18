#ifndef _COLORER_XSTR_H_
#define _COLORER_XSTR_H_

#include <ostream>
#include <memory>
#include <string>
#include <xercesc/util/XMLString.hpp>

/**
 * This is a simple class that lets us do easy trancoding of
 * XMLCh data to char* data.
 *
 */
class XStr
{
public:
  XStr(const XMLCh* const toTranscode);
  XStr(const std::string &toTranscode);
  XStr(const std::string* toTranscode);
  ~XStr();

  const char* get_char() const;
  const std::string* get_stdstr() const;
  const XMLCh* get_xmlchar() const;

private:
  XStr(XStr const &) = delete;
  XStr &operator=(XStr const &) = delete;
  XStr(XStr &&) = delete;
  XStr &operator=(XStr &&) = delete;

  friend std::ostream &operator<<(std::ostream &stream, const XStr &x);

  std::unique_ptr<XMLCh> _xmlch;
  std::string _string;
};

#endif //_COLORER_XSTR_H_
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
