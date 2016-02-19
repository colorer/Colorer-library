#ifndef _COLORER_XML_PARSER_ERROR_HANDLER_H_
#define _COLORER_XML_PARSER_ERROR_HANDLER_H_

#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/ErrorHandler.hpp>

/* XmlParserErrorHandler - class to catch errors and warnings from the XML Parser*/
class XmlParserErrorHandler : public xercesc::ErrorHandler
{
public:

  XmlParserErrorHandler(): fSawErrors(false)
  {
  }

  ~XmlParserErrorHandler()
  {
  }

  void warning(const xercesc::SAXParseException &toCatch) override;
  void error(const xercesc::SAXParseException &toCatch) override;
  void fatalError(const xercesc::SAXParseException &toCatch) override;
  void resetErrors() override;
  bool getSawErrors() const;

private:
  /* fSawErrors
  This is set if we get any errors, and is queryable via a getter
  method. Its used by the main code to suppress output if there are
  errors. */
  bool fSawErrors;
};

inline bool XmlParserErrorHandler::getSawErrors() const
{
  return fSawErrors;
}

inline void XmlParserErrorHandler::resetErrors()
{
  fSawErrors = false;
}

#endif  // _COLORER_XML_PARSER_ERROR_HANDLER_H_
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
