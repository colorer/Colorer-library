#include <colorer/parsers/TextParserImpl.h>

TextParserImpl::TextParserImpl()
{
  LOGF(TRACE, "[TextParserImpl] constructor");
  cache = new ParseCache();
  clearCache();
  lineSource = nullptr;
  regionHandler = nullptr;
  picked = nullptr;
  baseScheme = nullptr;
  memset(&matchend, 0, sizeof(SMatches));
}

TextParserImpl::~TextParserImpl()
{
  clearCache();
  delete cache;
}

void TextParserImpl::setFileType(FileType* type)
{
  baseScheme = nullptr;
  if (type != nullptr) {
    baseScheme = (SchemeImpl*)(type->getBaseScheme());
  }
  clearCache();
}

void TextParserImpl::setLineSource(LineSource* lh)
{
  lineSource = lh;
}

void TextParserImpl::setRegionHandler(RegionHandler* rh)
{
  regionHandler = rh;
}

int TextParserImpl::parse(int from, int num, TextParseMode mode)
{
  gx = 0;
  gy = from;
  gy2 = from + num;
  clearLine = -1;

  invisibleSchemesFilled = false;
  schemeStart = -1;
  breakParsing = false;
  updateCache = (mode == TPM_CACHE_UPDATE);

  LOGF(TRACE, "[TextParserImpl] parse from=%d, num=%d", from, num);
  /* Check for initial bad conditions */
  if (regionHandler == nullptr) {
    return from;
  }
  if (lineSource == nullptr) {
    return from;
  }
  if (baseScheme == nullptr) {
    return from;
  }

  vtlist = new VTList();

  lineSource->startJob(from);
  regionHandler->startParsing(from);

  /* Init cache */
  parent = cache;
  forward = nullptr;
  cache->scheme = baseScheme;

  if (mode == TPM_CACHE_READ || mode == TPM_CACHE_UPDATE) {
    parent = cache->searchLine(from, &forward);
    if (parent != nullptr) {
      LOGF(TRACE, "[TPCache] searchLine() parent:%s,%d-%d", parent->scheme->getName()->getChars(), parent->sline, parent->eline);
    }
  }
  cachedLineNo = from;
  cachedParent = parent;
  cachedForward = forward;
  LOGF(TRACE, "[TextParserImpl] parse: cache filled");


  do {
    if (!forward) {
      if (!parent) {
        return from;
      }
      if (updateCache) {
        delete parent->children;
        parent->children = nullptr;
      }
    } else {
      if (updateCache) {
        delete forward->next;
        forward->next = nullptr;
      }
    }
    baseScheme = parent->scheme;

    stackLevel = 0;
    LOGF(TRACE, "[TextParserImpl] parse: goes into colorize()");
    if (parent != cache) {
      vtlist->restore(parent->vcache);
      parent->clender->end->setBackTrace(parent->backLine, &parent->matchstart);
      colorize(parent->clender->end, parent->clender->lowContentPriority);
      vtlist->clear();
    } else {
      colorize(nullptr, false);
    }

    if (updateCache) {
      if (parent != cache) {
        parent->eline = gy;
      }
    }
    if (parent != cache && gy < gy2) {
      leaveScheme(gy, &matchend, parent->clender);
    }
    gx = matchend.e[0];

    forward = parent;
    parent = parent->parent;
  } while (parent);
  regionHandler->endParsing(endLine);
  lineSource->endJob(endLine);
  delete vtlist;
  return endLine;
}

void TextParserImpl::clearCache()
{
  ParseCache* tmp, *tmp2;
  tmp = cache->next;
  while (tmp) {
    tmp2 = tmp->next;
    delete tmp;
    tmp = tmp2;
  }
  delete cache->children;
  delete cache->backLine;
  cache->backLine = nullptr;
  cache->sline = 0;
  cache->eline = 0x7FFFFFF;
  cache->children = cache->parent = cache->next = nullptr;
}

void TextParserImpl::breakParse()
{
  breakParsing = true;
}

void TextParserImpl::addRegion(int lno, int sx, int ex, const Region* region)
{
  if (sx == -1 || region == nullptr) {
    return;
  }
  regionHandler->addRegion(lno, str, sx, ex, region);
}
void TextParserImpl::enterScheme(int lno, int sx, int ex, const Region* region)
{
  regionHandler->enterScheme(lno, str, sx, ex, region, baseScheme);
}
void TextParserImpl::leaveScheme(int lno, int sx, int ex, const Region* region)
{
  regionHandler->leaveScheme(lno, str, sx, ex, region, baseScheme);
  if (region != nullptr) {
    picked = region;
  }
}


void TextParserImpl::enterScheme(int lno, SMatches* match, const SchemeNode* schemeNode)
{
  int i;

  if (schemeNode->innerRegion == false) {
    enterScheme(lno, match->s[0], match->e[0], schemeNode->region);
  }

  for (i = 0; i < match->cMatch; i++) {
    addRegion(lno, match->s[i], match->e[i], schemeNode->regions[i]);
  }
  for (i = 0; i < match->cnMatch; i++) {
    addRegion(lno, match->ns[i], match->ne[i], schemeNode->regionsn[i]);
  }

  if (schemeNode->innerRegion == true) {
    enterScheme(lno, match->e[0], match->e[0], schemeNode->region);
  }
}

void TextParserImpl::leaveScheme(int lno, SMatches* match, const SchemeNode* schemeNode)
{
  int i;

  if (schemeNode->innerRegion == true) {
    leaveScheme(gy, match->s[0], match->s[0], schemeNode->region);
  }

  for (i = 0; i < match->cMatch; i++) {
    addRegion(gy, match->s[i], match->e[i], schemeNode->regione[i]);
  }
  for (i = 0; i < match->cnMatch; i++) {
    addRegion(gy, match->ns[i], match->ne[i], schemeNode->regionen[i]);
  }

  if (schemeNode->innerRegion == false) {
    leaveScheme(gy, match->s[0], match->e[0], schemeNode->region);
  }
}

void TextParserImpl::fillInvisibleSchemes(ParseCache* ch)
{
  if (!ch->parent || ch == cache) {
    return;
  }
  /* Fills output stream with valid "pseudo" enterScheme */
  fillInvisibleSchemes(ch->parent);
  enterScheme(gy, 0, 0, ch->clender->region);
  return;
}

int TextParserImpl::searchKW(const SchemeNode* node, int no, int lowlen, int hilen)
{
  if (!node->kwList->num) {
    return MATCH_NOTHING;
  }

  if (node->kwList->minKeywordLength + gx > lowlen) {
    return MATCH_NOTHING;
  }
  if (gx < lowlen && !node->kwList->firstChar->inClass((*str)[gx])) {
    return MATCH_NOTHING;
  }

  int left = 0;
  int right = node->kwList->num;
  while (true) {
    int pos = left + (right - left) / 2;
    int kwlen = node->kwList->kwList[pos].keyword->length();
    if (lowlen < gx + kwlen) {
      kwlen = lowlen - gx;
    }

    int cr;
    if (node->kwList->matchCase) {
      cr = node->kwList->kwList[pos].keyword->compareTo(DString(*str, gx, kwlen));
    } else {
      cr = node->kwList->kwList[pos].keyword->compareToIgnoreCase(DString(*str, gx, kwlen));
    }

    if (cr == 0 && right - left == 1) {
      bool badbound = false;
      if (!node->kwList->kwList[pos].isSymbol) {
        if (!node->worddiv) {
          if (gx && (Character::isLetterOrDigit((*str)[gx - 1]) || (*str)[gx - 1] == '_')) {
            badbound = true;
          }
          if (gx + kwlen < lowlen &&
              (Character::isLetterOrDigit((*str)[gx + kwlen]) || (*str)[gx + kwlen] == '_')) {
            badbound = true;
          }
        } else {
          // custom check for word bound
          if (gx && !node->worddiv->inClass((*str)[gx - 1])) {
            badbound = true;
          }
          if (gx + kwlen < lowlen &&
              !node->worddiv->inClass((*str)[gx + kwlen])) {
            badbound = true;
          }
        }
      }
      if (!badbound) {
        LOGF(TRACE, "[TextParserImpl] KW matched. gx=%d, region=%s", gx, node->kwList->kwList[pos].region->getName()->getChars());
        addRegion(gy, gx, gx + kwlen, node->kwList->kwList[pos].region);
        gx += kwlen;
        return MATCH_RE;
      }
    }
    if (right - left == 1) {
      left = node->kwList->kwList[pos].ssShorter;
      if (left != -1) {
        right = left + 1;
        continue;
      }
      break;
    }
    if (cr == 1) {
      right = pos;
    }
    if (cr == 0 || cr == -1) {
      left = pos;
    }
  }
  return MATCH_NOTHING;
}

int TextParserImpl::searchRE(SchemeImpl* cscheme, int no, int lowLen, int hiLen)
{
  int i, re_result;
  SchemeImpl* ssubst = nullptr;
  SMatches match;
  ParseCache* OldCacheF = nullptr;
  ParseCache* OldCacheP = nullptr;
  ParseCache* ResF = nullptr;
  ParseCache* ResP = nullptr;

  LOGF(TRACE, "[TextParserImpl] searchRE: entered scheme \"%s\"", cscheme->getName()->getChars());

  if (!cscheme) {
    return MATCH_NOTHING;
  }
  for (int idx = 0; idx < cscheme->nodes.size(); idx++) {
    SchemeNode* schemeNode = cscheme->nodes.at(idx);
    LOGF(TRACE, "[TextParserImpl] searchRE: processing node:%d/%d, type:%s", idx + 1, cscheme->nodes.size(), schemeNodeTypeNames[schemeNode->type]);
    switch (schemeNode->type) {
      case SNT_INHERIT:
        if (!schemeNode->scheme) {
          break;
        }
        ssubst = vtlist->pushvirt(schemeNode->scheme);
        if (!ssubst) {
          bool b = vtlist->push(schemeNode);
          re_result = searchRE(schemeNode->scheme, no, lowLen, hiLen);
          if (b) {
            vtlist->pop();
          }
        } else {
          re_result = searchRE(ssubst, no, lowLen, hiLen);
          vtlist->popvirt();
        }
        if (re_result != MATCH_NOTHING) {
          return re_result;
        }
        break;

      case SNT_KEYWORDS:
        if (searchKW(schemeNode, no, lowLen, hiLen) == MATCH_RE) {
          return MATCH_RE;
        }
        break;

      case SNT_RE:
        if (!schemeNode->start->parse(str, gx, schemeNode->lowPriority ? lowLen : hiLen, &match, schemeStart)) {
          break;
        }
        LOGF(TRACE, "[TextParserImpl] RE matched. gx=%d", gx);
        for (i = 0; i < match.cMatch; i++) {
          addRegion(gy, match.s[i], match.e[i], schemeNode->regions[i]);
        }
        for (i = 0; i < match.cnMatch; i++) {
          addRegion(gy, match.ns[i], match.ne[i], schemeNode->regionsn[i]);
        }

        /* skips regexp if it has zero length */
        if (match.e[0] == match.s[0]) {
          break;
        }
        gx = match.e[0];
        return MATCH_RE;

      case SNT_SCHEME: {
        if (!schemeNode->scheme) {
          break;
        }
        if (!schemeNode->start->parse(str, gx,
                                      schemeNode->lowPriority ? lowLen : hiLen, &match, schemeStart)) {
          break;
        }

        LOGF(TRACE, "[TextParserImpl] Scheme matched. gx=%d", gx);

        gx = match.e[0];
        ssubst = vtlist->pushvirt(schemeNode->scheme);
        if (!ssubst) {
          ssubst = schemeNode->scheme;
        }

        SString* backLine = new SString(str);
        if (updateCache) {
          ResF = forward;
          ResP = parent;
          if (forward) {
            forward->next = new ParseCache;
            forward->next->prev = forward;
            OldCacheF = forward->next;
            OldCacheP = parent ? parent : forward->parent;
            parent = forward->next;
            forward = nullptr;
          } else {
            forward = new ParseCache;
            parent->children = forward;
            OldCacheF = forward;
            OldCacheP = parent;
            parent = forward;
            forward = nullptr;
          }
          OldCacheF->parent = OldCacheP;
          OldCacheF->sline = gy + 1;
          OldCacheF->eline = 0x7FFFFFFF;
          OldCacheF->scheme = ssubst;
          OldCacheF->matchstart = match;
          OldCacheF->clender = schemeNode;
          OldCacheF->backLine = backLine;
        }

        int ogy = gy;
        bool zeroLength;

        SchemeImpl* o_scheme = baseScheme;
        int o_schemeStart = schemeStart;
        SMatches o_matchend = matchend;
        SMatches* o_match;
        DString* o_str;
        schemeNode->end->getBackTrace((const String**)&o_str, &o_match);

        baseScheme = ssubst;
        schemeStart = gx;
        schemeNode->end->setBackTrace(backLine, &match);

        enterScheme(no, &match, schemeNode);

        colorize(schemeNode->end, schemeNode->lowContentPriority);

        if (gy < gy2) {
          leaveScheme(gy, &matchend, schemeNode);
        }
        gx = matchend.e[0];
        /* (empty-block.test) Check if the consumed scheme is zero-length */
        zeroLength = (match.s[0] == matchend.e[0] && ogy == gy);

        schemeNode->end->setBackTrace(o_str, o_match);
        matchend = o_matchend;
        schemeStart = o_schemeStart;
        baseScheme = o_scheme;

        if (updateCache) {
          if (ogy == gy) {
            delete OldCacheF;
            if (ResF) {
              ResF->next = nullptr;
            } else {
              ResP->children = nullptr;
            }
            forward = ResF;
            parent = ResP;
          } else {
            OldCacheF->eline = gy;
            OldCacheF->vcache = vtlist->store();
            forward = OldCacheF;
            parent = OldCacheP;
          }
        } else {
          delete backLine;
        }
        if (ssubst != schemeNode->scheme) {
          vtlist->popvirt();
        }
        /* (empty-block.test) skips block if it has zero length and spread over single line */
        if (zeroLength) {
          break;
        }

        return MATCH_SCHEME;
      }
    }
  }
  return MATCH_NOTHING;
}

bool TextParserImpl::colorize(CRegExp* root_end_re, bool lowContentPriority)
{
  len = -1;

  /* Direct check for recursion level */
  if (stackLevel > MAX_RECURSION_LEVEL) {
    return true;
  }
  stackLevel++;

  for (; gy < gy2;) {
    LOGF(TRACE, "[TextParserImpl] colorize: line no %d", gy);
    // clears line at start,
    // prevents multiple requests on each line
    if (clearLine != gy) {
      clearLine = gy;
      str = lineSource->getLine(gy);
      if (str == nullptr) {
        throw Exception(StringBuffer("null String passed into the parser: ") + SString(gy));
        //!!unreachable code
        //gy = gy2;
        break;
      }
      regionHandler->clearLine(gy, str);
    }
    // hack to include invisible regions in start of block
    // when parsing with cache information
    if (!invisibleSchemesFilled) {
      invisibleSchemesFilled = true;
      fillInvisibleSchemes(parent);
    }
    // updates length
    if (len < 0) {
      len = str->length();
    }
    endLine = gy;

    // searches for the end of parent block
    int res = 0;
    if (root_end_re) {
      res = root_end_re->parse(str, gx, len, &matchend, schemeStart);
    }
    if (!res) {
      matchend.s[0] = matchend.e[0] = len;
    }

    int parent_len = len;
    /*
    BUG: <regexp match="/.{3}\M$/" region="def:Error" priority="low"/>
    $ at the end of current schema
    */
    if (lowContentPriority) {
      len = matchend.s[0];
    }

    int ret = LINE_NEXT;
    for (; gx <= matchend.s[0];) { //    '<' or '<=' ???
      if (breakParsing) {
        gy = gy2;
        break;
      }
      if (picked != nullptr && gx + 11 <= matchend.s[0] && (*str)[gx] == 'C') {
        int ci;
        static char id[] = "fnq%Qtrjhg";
        for (ci = 0; ci < 10; ci++) if ((*str)[gx + 1 + ci] != id[ci] - 5) {
            break;
          }
        if (ci == 10) {
          addRegion(gy, gx, gx + 11, picked);
          gx += 11;
          continue;
        }
      }
      int oy = gy;
      int re_result = searchRE(baseScheme, gy, matchend.s[0], len);
      if ((re_result == MATCH_SCHEME && (oy != gy || matchend.s[0] < gx)) ||
          (re_result == MATCH_RE && matchend.s[0] < gx)) {
        len = -1;
        ret = LINE_REPARSE;
        break;
      }
      if (oy == gy) {
        len = parent_len;
      }
      if (re_result == MATCH_NOTHING) {
        gx++;
      }
    }
    if (ret == LINE_REPARSE) {
      continue;
    }

    schemeStart = -1;
    if (res) {
      stackLevel--;
      return true;
    }
    len = -1;
    gy++;
    gx = 0;
  }
  stackLevel--;
  return true;
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

