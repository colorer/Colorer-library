#ifndef _COLORER_OUTLINEITEM_H_
#define _COLORER_OUTLINEITEM_H_

#include <memory>
#include <colorer/Region.h>

/**
 * Item in outliner's list.
 * Contans all the information about single
 * structured token with specified type (region reference).
 * @ingroup colorer_editor
 */
class OutlineItem
{
public:
  /** Line number */
  size_t lno;
  /** Position in line */
  int pos;
  /** Level of enclosure */
  int level;
  /** Item text */
  std::unique_ptr<SString> token;
  /** This item's region */
  const Region* region;

  /** Default constructor */
  OutlineItem() : lno(0), pos(0), level(0), token(nullptr), region(nullptr)
  {
  }

  /** Initializing constructor */
  OutlineItem(size_t lno_, int pos_, int level_, String* token_, const Region* region_):
    lno(lno_), pos(pos_), level(level_), token(nullptr), region(region_)
  {
    if (token_ != nullptr) {
      token.reset(new SString(token_));
    }
  }

  ~OutlineItem()
  {
  }
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

