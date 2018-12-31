#include <colorer/handlers/LineRegionsSupport.h>

LineRegionsSupport::LineRegionsSupport()
{
  lineCount = 0;
  firstLineNo = 0;
  regionMapper = nullptr;
  special = nullptr;
}

LineRegionsSupport::~LineRegionsSupport()
{
  clear();
  for (size_t i = 1; i < schemeStack.size();i++){
    delete schemeStack[i];
  }
  schemeStack.clear();
}

void LineRegionsSupport::resize(size_t lineCount_)
{
  lineRegions.resize(lineCount_);
  this->lineCount = lineCount_;
}

size_t LineRegionsSupport::size()
{
  return lineCount;
}

void LineRegionsSupport::clear()
{
  for (size_t idx = 0; idx < lineRegions.size(); idx++) {
    LineRegion* ln = lineRegions.at(idx);
    lineRegions.at(idx) = nullptr;
    while (ln != nullptr) {
      LineRegion* lnn = ln->next;
      delete ln;
      ln = lnn;
    }
  }
}

size_t LineRegionsSupport::getLineIndex(size_t lno) const
{
  return ((firstLineNo % lineCount) + lno - firstLineNo) % lineCount;
}

LineRegion* LineRegionsSupport::getLineRegions(size_t lno) const
{
  if (!checkLine(lno)) {
    return nullptr;
  }
  return lineRegions.at(getLineIndex(lno));
}

void LineRegionsSupport::setFirstLine(size_t first)
{
  firstLineNo = first;
}

size_t LineRegionsSupport::getFirstLine()
{
  return firstLineNo;
}

void LineRegionsSupport::setBackground(const RegionDefine* back)
{
  background.rdef = const_cast<RegionDefine*>(back);
}

void LineRegionsSupport::setSpecialRegion(const Region* special)
{
  this->special = special;
}

void LineRegionsSupport::setRegionMapper(const RegionMapper* rs)
{
  regionMapper = rs;
}

bool LineRegionsSupport::checkLine(size_t lno) const
{
  if (lno < firstLineNo || lno >= firstLineNo + lineCount) {
    LOGF(WARNING, "[LineRegionsSupport] checkLine: line %zd out of range", lno);
    return false;
  }
  return true;
}

void LineRegionsSupport::startParsing(size_t lno)
{
  for (size_t i = 1; i < schemeStack.size(); i++) {
    delete schemeStack[i];
  }
  schemeStack.clear();
  schemeStack.push_back(&background);
}

void LineRegionsSupport::clearLine(size_t lno, String* line)
{
  if (!checkLine(lno)) {
    return;
  }

  LineRegion* ln = getLineRegions(lno);
  while (ln != nullptr) {
    LineRegion* lnn = ln->next;
    delete ln;
    ln = lnn;
  }
  LineRegion* lfirst = new LineRegion(*schemeStack.back());
  lfirst->start = 0;
  lfirst->end = -1;
  lfirst->next = nullptr;
  lfirst->prev = lfirst;
  lineRegions.at(getLineIndex(lno)) = lfirst;
  flowBackground = lfirst;
}

void LineRegionsSupport::addRegion(size_t lno, String* line, int sx, int ex, const Region* region)
{
  // ignoring out of cached interval lines
  if (!checkLine(lno)) {
    return;
  }
  LineRegion* lnew = new LineRegion();
  lnew->start = sx;
  lnew->end = ex;
  lnew->region = region;
  lnew->scheme = schemeStack.back()->scheme;
  if (region->hasParent(special)) {
    lnew->special = true;
  }
  if (regionMapper != nullptr) {
    const RegionDefine* rd = regionMapper->getRegionDefine(region);
    if (rd == nullptr) {
      rd = schemeStack.back()->rdef;
    }
    if (rd != nullptr) {
      lnew->rdef = rd->clone();
      lnew->rdef->assignParent(schemeStack.back()->rdef);
    }
  }
  addLineRegion(lno, lnew);
}

void LineRegionsSupport::enterScheme(size_t lno, String* line, int sx, int ex, const Region* region, const Scheme* scheme)
{
  LineRegion* lr = new LineRegion();
  lr->region = region;
  lr->scheme = scheme;
  lr->start = sx;
  lr->end = -1;
  if (regionMapper != nullptr) {
    const RegionDefine* rd = regionMapper->getRegionDefine(region);
    if (rd == nullptr) {
      rd = schemeStack.back()->rdef;
    }
    if (rd != nullptr) {
      lr->rdef = rd->clone();
      lr->rdef->assignParent(schemeStack.back()->rdef);
    }
  }
  schemeStack.push_back(lr);
  // ignoring out of cached interval lines
  if (!checkLine(lno)) {
    return;
  }
  // we must skip transparent regions
  if (lr->region != nullptr) {
    LineRegion* lr_add = new LineRegion(*lr);
    flowBackground->end = lr_add->start;
    flowBackground = lr_add;
    addLineRegion(lno, lr_add);
  }
}

void LineRegionsSupport::leaveScheme(size_t lno, String* line, int sx, int ex, const Region* region, const Scheme* scheme)
{
  const Region* scheme_region = schemeStack.back()->region;
  delete schemeStack.back();
  schemeStack.pop_back();
  // ignoring out of cached interval lines
  if (!checkLine(lno)) {
    return;
  }
  // we have to skip transparent regions
  if (scheme_region != nullptr) {
    LineRegion* lr = new LineRegion(*schemeStack.back());
    lr->start = ex;
    lr->end = -1;
    flowBackground->end = lr->start;
    flowBackground = lr;
    addLineRegion(lno, lr);
  }
}

void LineRegionsSupport::addLineRegion(size_t lno, LineRegion* lr)
{
  LineRegion* lstart = getLineRegions(lno);
  lr->next = nullptr;
  lr->prev = lr;
  if (lstart == nullptr) {
    lineRegions.at(getLineIndex(lno)) = lr;
  } else {
    lr->prev = lstart->prev;
    lr->prev->next = lr;
    lstart->prev = lr;
  }
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
