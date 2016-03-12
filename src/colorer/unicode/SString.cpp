#include <stdio.h>
#include <colorer/unicode/SString.h>
#include <colorer/unicode/DString.h>

SString::SString()
{
  alloc = 0;
  wstr = nullptr;
  len = 0;
}

void SString::construct(const String* cstring, size_t s, size_t l)
{
  if (s > cstring->length()) throw Exception(DString("bad string constructor parameters"));
  if (l == -1) l = cstring->length() - s;
  wstr.reset(new wchar[l]);
  for (len = 0; len < l; len++)
    wstr[len] = (*cstring)[s + len];
  alloc = len;
}

SString::SString(const String* cstring, size_t s, size_t l)
{
  construct(cstring, s, l);
}

SString::SString(const SString &cstring)
{
  construct(&cstring, 0, npos);
}

SString::SString(const char* string, size_t s, size_t l)
{
  DString ds(string, s, l);
  construct(&ds, 0, ds.length());
}

SString::SString(const wchar* string, size_t s, size_t l)
{
  DString ds(string, s, l);
  construct(&ds, 0, ds.length());
}

SString::SString(const String &cstring, size_t s, size_t l)
{
  construct(&cstring, s, l);
}

SString::SString(char* str, int enc)
{
  DString ds(str, 0, -1, enc);
  construct(&ds, 0, ds.length());
}

SString::SString(wchar* str)
{
  DString ds(str, 0, -1);
  construct(&ds, 0, ds.length());
}

SString::SString(int no)
{
  char text[40];
  sprintf(text, "%d", no);
  DString dtext = DString(text);
  construct(&dtext, 0, npos);
}

SString::SString(size_t no)
{
  char text[40];
  sprintf(text, "%zd", no); //-V111
  DString dtext = DString(text);
  construct(&dtext, 0, npos);
}

SString::~SString()
{

}

void SString::setLength(size_t newLength)
{
  if (newLength > alloc) {
    std::unique_ptr<wchar[]> wstr_new(new wchar[newLength * 2]);
    alloc = newLength * 2;
    for (size_t i = 0; i < newLength; i++) {
      if (i < len) wstr_new[i] = wstr[i];
      else wstr_new[i] = 0;
    };
    wstr = std::move(wstr_new);
  }
  len = newLength;
}

wchar SString::operator[](size_t i) const
{
  return wstr[i];
}

SString &SString::append(const String* string)
{
  if (string == nullptr)
    return append(DString("null"));
  return append(*string);
}

SString &SString::append(const String &string)
{
  size_t len_new = len + string.length();
  if (alloc > len_new) {
    for (size_t i = len; i < len_new; i++)
      wstr[i] = string[i - len];
  } else {
    std::unique_ptr<wchar[]> wstr_new(new wchar[len_new * 2]);
    alloc = len_new * 2;
    for (size_t i = 0; i < len_new; i++) {
      if (i < len) wstr_new[i] = wstr[i];
      else wstr_new[i] = string[i - len];
    };
    wstr = std::move(wstr_new);
  };
  len = len_new;
  return *this;
}

SString &SString::append(wchar c)
{
  setLength(len + 1);
  wstr[len - 1] = c;
  return *this;
}

SString &SString::operator+(const String &string)
{
  return append(string);
}

SString &SString::operator+(const String* string)
{
  return append(string);
}

SString &SString::operator+(const char* string)
{
  return append(DString(string));
}

SString &SString::operator+=(const char* string)
{
  return operator+(DString(string));
}

SString &SString::operator+=(const String &string)
{
  return operator+(string);
}

SString &SString::operator=(SString const &cstring)
{
  construct(&cstring, 0, -1);
  return *this;
}

SString* SString::replace(const String &pattern, const String &newstring) const
{
  size_t copypos = 0;
  size_t epos = 0;

  SString* newname = new SString();
  const String &name = *this;

  while (true) {
    epos = name.indexOf(pattern, epos);
    if (epos == -1) {
      epos = name.length();
      break;
    }
    newname->append(DString(name, copypos, epos - copypos));
    newname->append(newstring);
    epos = epos + pattern.length();
    copypos = epos;
  }
  if (epos > copypos) newname->append(DString(name, copypos, epos - copypos));
  return newname;
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

