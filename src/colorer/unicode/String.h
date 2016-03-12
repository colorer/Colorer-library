#ifndef _COLORER_STRING_H_
#define _COLORER_STRING_H_

#include <colorer/unicode/CommonString.h>

#ifdef __unix__
extern "C" int stricmp(const char*c1, const char*c2);
extern "C" int strnicmp(const char*c1, const char*c2, unsigned int n);
#endif

/** Abstract unicode string class.
    @ingroup unicode
*/
class String{
public:
  String();
  virtual ~String();

  virtual wchar operator[](size_t i) const = 0;
  /** String length in unicode characters */
  virtual size_t length() const = 0;

  /** Checks, if two strings are equals */
  bool operator==(const String &str) const;
  /** Checks, if two strings are not equals */
  bool operator!=(const String &str) const;
  /** Checks, if this string lexically 'more', than @c str */
  bool operator>(const String &str) const;
  /** Checks, if this string lexically 'less', than @c str */
  bool operator<(const String &str) const;

  /** Checks, if two strings are equals */
  bool equals(const String *str) const;
  /** Checks, if two strings are equals, ignoring Case Folding */
  bool equalsIgnoreCase(const String *str) const;
  /** Compares two strings.
      @return -1 if this < str;
              0  if this == str;
              1  if this > str;
  */
  int compareTo(const String &str) const;
  /** Compares two strings ignoring case
      @return -1 if this < str;
              0  if this == str;
              1  if this > str;
  */
  int compareToIgnoreCase(const String &str) const;
  /** Copies content of string into array of wchars */
  virtual size_t getWChars(wchar **chars) const;
  /** Copies content of string into array of bytes, using specified encoding */
  virtual size_t getBytes(byte **bytes, int encoding = -1) const;
  /** Returns string content in internally supported character array */
  virtual const char *getChars(int encoding = -1) const;
  /** Returns string content in internally supported unicode character array */
  virtual const wchar *getWChars() const;

  /** Searches first index of char @c wc, starting from @c pos */
  virtual size_t indexOf(wchar wc, size_t pos = 0) const;
  /** Searches first index of substring @c str, starting from @c pos */
  virtual size_t indexOf(const String &str, size_t pos = 0) const;
  /** Searches first index of substring @c str, starting from @c pos ignoring character case */
  virtual size_t indexOfIgnoreCase(const String &str, size_t pos = 0) const;
  /** Searches last index of character @c wc, ending with @c pos */
  virtual size_t lastIndexOf(wchar wc, size_t pos = npos) const;
  /** Searches last index of substring @c str, ending with @c pos */
  virtual size_t lastIndexOf(const String &str, size_t pos = npos) const;

  /** Tests, if string starts with specified @c str substring at position @c pos */
  virtual bool startsWith(const String &str, size_t pos = 0) const;

  /** Internal hashcode of string
  */
  size_t hashCode() const;

  static const size_t npos = size_t(-1);
private:
  char *ret_char_val;
  wchar *ret_wchar_val;
};
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

