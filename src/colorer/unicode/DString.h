#ifndef _COLORER_DSTRING_H_
#define _COLORER_DSTRING_H_

#include <colorer/unicode/SString.h>
#include <colorer/unicode/CString.h>

/** Dynamic string class.
    Simple unicode wrapper over any other source.
    @ingroup unicode
*/
class DString : public SString
{
public:
  /** String from any @c String implementing interface.
      @param cstring String class instance, can't be null.
      @param s Starting string position.
      @param l Length of created string. If npos, autodetects string length with
             cstring.length() call.
  */
  DString(const SString* cstring, size_t s = 0, size_t l = npos);

  /** String from any @c String implementing interface.
      @param cstring String class instance.
      @param s Starting string position.
      @param l Length of created string. If npos, autodetects string length with
             cstring.length() call.
  */
  DString(const SString &cstring, size_t s = 0, size_t l = npos);

  ~DString() {};

  DString(DString const &) = delete;
  DString &operator=(DString const &) = delete;
  DString(DString &&);
  DString &operator=(DString &&);

  wchar operator[](size_t i) const override;
  size_t length() const override;

protected:
  const SString* str;
  size_t start;
  size_t len;
};

inline DString::DString(const SString* cstring, size_t s, size_t l)
{
  str = cstring;
  start = s;
  len = l;
  if (s > cstring->length() || (len != npos && len > cstring->length() - start))
    throw Exception(CString("bad string constructor parameters"));
  if (len == npos)
    len = cstring->length() - start;
}

inline DString::DString(const SString &cstring, size_t s, size_t l)
{
  str = &cstring;
  start = s;
  len = l;
  if (s > cstring.length() || (len != npos && len > cstring.length() - start))
    throw Exception(CString("bad string constructor parameters"));
  if (len == npos)
    len = cstring.length() - start;
}

inline DString::DString(DString &&cstring):
  str(cstring.str),
  start(cstring.start),
  len(cstring.len)
{
  cstring.str = nullptr;
  cstring.len = 0;
  cstring.start = 0;
}

inline DString &DString::operator=(DString &&cstring)
{
  if (this != &cstring) {
    str = cstring.str;
    len = cstring.len;
    start = cstring.start;

    cstring.str = nullptr;
    cstring.len = 0;
    cstring.start = 0;
  }
  return *this;
}

inline wchar DString::operator[](size_t i) const
{
  return (*str)[start + i];
}

inline size_t DString::length() const
{
  return len;
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

