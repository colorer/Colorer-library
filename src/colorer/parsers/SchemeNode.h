#ifndef _COLORER_SCHEMENODE_H_
#define _COLORER_SCHEMENODE_H_

#include <colorer/Common.h>
#include <colorer/Region.h>
#include <colorer/parsers/KeywordList.h>
#include <colorer/parsers/VirtualEntry.h>
#include <colorer/cregexp/cregexp.h>

enum SchemeNodeType { SNT_EMPTY, SNT_RE, SNT_SCHEME, SNT_KEYWORDS, SNT_INHERIT };
extern const char* schemeNodeTypeNames[];

class SchemeImpl;
typedef std::vector<VirtualEntry*> VirtualEntryVector;

// Must be not less than MATCHES_NUM in cregexp.h
#define REGIONS_NUM MATCHES_NUM
#define NAMED_REGIONS_NUM NAMED_MATCHES_NUM

/** Scheme node.
    @ingroup colorer_parsers
*/
class SchemeNode
{
public:
  SchemeNodeType type;

  UString schemeName;
  SchemeImpl* scheme;

  VirtualEntryVector virtualEntryVector;
  KeywordList* kwList;
  CharacterClass* worddiv;

  const Region* region;
  const Region* regions[REGIONS_NUM];
  const Region* regionsn[NAMED_REGIONS_NUM];
  const Region* regione[REGIONS_NUM];
  const Region* regionen[NAMED_REGIONS_NUM];
  CRegExp* start, *end;
  bool innerRegion, lowPriority, lowContentPriority;

#include<colorer/common/MemoryOperator.h>

  SchemeNode();
  ~SchemeNode();
};


#endif //_COLORER_SCHEMENODE_H_

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
