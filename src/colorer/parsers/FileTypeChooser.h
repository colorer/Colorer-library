#ifndef _COLORER_FILETYPECHOOSER_H_
#define _COLORER_FILETYPECHOOSER_H_

#include <colorer/cregexp/cregexp.h>

/** Stores regular expressions of filename and firstline
    elements and helps to detect file type.
    @ingroup colorer_parsers
*/
class FileTypeChooser
{
public:
  enum ChooserType { CT_FILENAME, CT_FIRSTLINE };

  /** Creates choose entry.
      @param type If 0 - filename RE, if 1 - firstline RE
      @param prior Priority of this rule
      @param re Associated regular expression
  */
  FileTypeChooser(ChooserType type, double prior, CRegExp* re);
  /** Default destructor */
  ~FileTypeChooser() {};
  /** Returns type of chooser */
  bool isFileName() const;
  /** Returns type of chooser */
  bool isFileContent() const;
  /** Returns chooser priority */
  double getPriority() const;
  /** Returns associated regular expression */
  CRegExp* getRE() const;
private:
  std::unique_ptr<CRegExp> reg_matcher;
  ChooserType type;
  double priority;
};

inline FileTypeChooser::FileTypeChooser(ChooserType type_, double prior, CRegExp* re):
  reg_matcher(re), type(type_), priority(prior)
{
}

inline bool FileTypeChooser::isFileName() const
{
  return type == CT_FILENAME;
}

inline bool FileTypeChooser::isFileContent() const
{
  return type == CT_FIRSTLINE;
}

inline double FileTypeChooser::getPriority() const
{
  return priority;
}

inline CRegExp* FileTypeChooser::getRE() const
{
  return reg_matcher.get();
}

#endif //_COLORER_FILETYPECHOOSER_H_

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
