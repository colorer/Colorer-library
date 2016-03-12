#ifndef _COLORER_EXCEPTION_H__
#define _COLORER_EXCEPTION_H__

#include <exception>
#include <string>

class String;

/** Exception class.
    Defines throwable exception.
    @ingroup common
*/
class Exception: public std::exception
{
public:
  /** Default constructor
      Creates exception with empty message
  */
  Exception() noexcept;
  Exception(const std::string &msg) noexcept;
  /** Creates exception with string message
  */
  Exception(const String &msg) noexcept;
  /** Creates exception with included exception information
  */
  Exception(const Exception &e) noexcept;
  Exception &operator =(const Exception &e) noexcept;
  /** Default destructor
  */
  virtual ~Exception() noexcept;

  /** Returns exception message
  */
  virtual const char* what() const noexcept override;
protected:
  /** Internal message container
  */
  std::string what_str;
};

/**
    InputSourceException is thrown, if some IO error occurs.
    @ingroup common
*/
class InputSourceException : public Exception{
public:
  InputSourceException() noexcept;
  InputSourceException(const std::string& msg) noexcept;
  InputSourceException(const String& msg) noexcept;
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
