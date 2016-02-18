#ifndef _COLORER_LINEREGIONSSUPPORT_H_
#define _COLORER_LINEREGIONSSUPPORT_H_

#include <colorer/RegionHandler.h>
#include <colorer/handlers/RegionDefine.h>
#include <colorer/handlers/RegionMapper.h>
#include <colorer/handlers/LineRegion.h>

/** Region store implementation of RegionHandler.
    @ingroup colorer_handlers
*/
class LineRegionsSupport : public RegionHandler
{
public:
  LineRegionsSupport();
  ~LineRegionsSupport();

  /**
   * Resizes structures to mantain regions for @c lineCount lines.
   */
  void resize(size_t lineCount);

  /**
   * Return current size of this line regions structure
   */
  size_t size();

  /**
   * Drops all internal structures
   */
  void clear();

  /**
   * Sets start line position of line structures.
   * This position tells, that first line structure refers
   * not to first line of text, but to @c first parameter value.
   */
  void setFirstLine(size_t first);

  /**
   * Returns first line position, installed in this line structures.
   */
  size_t getFirstLine();

  /**
   * Background region define, which is used to
   * fill transparent regions. If background is @c null,
   * then regions with transparent fields would leave these fields unfilled
   */
  void setBackground(const RegionDefine* back);

  /**
   * Tells handler to mark with special field
   * all Regions with specified ancestor.
   */
  void setSpecialRegion(const Region* special);

  /**
   * Choose the source of RegionDefine definitions.
   * This source returns information about mapping
   * Region objects into RegionDefine objects.
   */
  void setRegionMapper(const RegionMapper* rds);

  /**
   * Returns LineRegion object for @c lno line number.
   * This object is linked with all other stored @c LineRegion objects
   */
  LineRegion* getLineRegions(size_t lno) const;

  /**
   * RegionHandler implementation
   */
  void startParsing(size_t lno);
  void clearLine(size_t lno, String* line);
  void addRegion(size_t lno, String* line, int sx, int ex, const Region* region);
  void enterScheme(size_t lno, String* line, int sx, int ex, const Region* region, const Scheme* scheme);
  void leaveScheme(size_t lno, String* line, int sx, int ex, const Region* region, const Scheme* scheme);
protected:
  /**
   * Behaviour is redefined in derived classes
   */
  virtual void addLineRegion(size_t lno, LineRegion* lr);
  size_t getLineIndex(size_t lno) const;
  bool checkLine(size_t lno) const;

  std::vector<LineRegion*> lineRegions;
  std::vector<LineRegion*> schemeStack;

  const RegionMapper* regionMapper;
  LineRegion* flowBackground;
  const Region* special;

  LineRegion background;
  size_t firstLineNo;
  size_t lineCount;
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

