#include <colorer/editor/Outliner.h>

Outliner::Outliner(BaseEditor* baseEditor, const Region* searchRegion)
{
  this->searchRegion = searchRegion;
  modifiedLine = -1;
  this->baseEditor = baseEditor;
  baseEditor->addRegionHandler(this);
  baseEditor->addEditorListener(this);
}

Outliner::~Outliner()
{
  baseEditor->removeRegionHandler(this);
  baseEditor->removeEditorListener(this);

  for (auto it : outline) {
    delete it;
  }
  outline.clear();
}

OutlineItem* Outliner::getItem(size_t idx)
{
  return outline.at(idx);
}

size_t Outliner::itemCount()
{
  return outline.size();
}

int Outliner::manageTree(std::vector<int>& treeStack, int newLevel)
{
  while (treeStack.size() > 0 && newLevel < treeStack.back()) {
    treeStack.pop_back();
  }
  if (treeStack.size() == 0 || newLevel > treeStack.back()) {
    treeStack.push_back(newLevel);
    return treeStack.size() - 1;
  }
  if (newLevel == treeStack.back()) {
    return treeStack.size() - 1;
  }
  return 0;
}

bool Outliner::isOutlined(const Region* region)
{
  return region->hasParent(searchRegion);
}

void Outliner::modifyEvent(size_t topLine)
{
  for (size_t i = 0; i <outline.size(); ++i) {
    if (outline[i]->lno > topLine) {
      outline.resize(i);
      break;
    }
  }

  modifiedLine = topLine;
}

void Outliner::startParsing(size_t lno)
{
  curLevel = 0;
}

void Outliner::endParsing(size_t lno)
{
  if (modifiedLine < lno) {
    modifiedLine = lno + 1;
  }
  curLevel = 0;
}

void Outliner::clearLine(size_t lno, String* line)
{
  lineIsEmpty = true;
}

void Outliner::addRegion(size_t lno, String* line, int sx, int ex, const Region* region)
{
  if (lno < modifiedLine) {
    return;
  }
  if (!isOutlined(region)) {
    return;
  }

  String* itemLabel = new CString(line, sx, ex - sx);

  if (lineIsEmpty) {
    outline.push_back(new OutlineItem(lno, sx, curLevel, itemLabel, region));
  } else {
    OutlineItem* thisItem = outline.back();
    if (thisItem->token != nullptr && thisItem->lno == lno) {
      thisItem->token->append(itemLabel);
    }
  }
  delete itemLabel;
  lineIsEmpty = false;
}

void Outliner::enterScheme(size_t lno, String* line, int sx, int ex, const Region* region, const Scheme* scheme)
{
  curLevel++;
}

void Outliner::leaveScheme(size_t lno, String* line, int sx, int ex, const Region* region, const Scheme* scheme)
{
  curLevel--;
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

