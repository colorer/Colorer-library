#include <colorer/parsers/KeywordList.h>
/*
KeywordInfo::KeywordInfo()
{
  keyword = null;
  ssShorter = -1;
  isSymbol = false;
  region = null;
};
void KeywordInfo::swapWith(KeywordInfo* kwi)
{
  const SString* _keyword = keyword;
  bool _isSymbol = isSymbol;
  const Region* _region = region;
  int _ssShorter = ssShorter;
  keyword   = kwi->keyword;
  isSymbol  = kwi->isSymbol;
  region    = kwi->region;
  ssShorter = kwi->ssShorter;

  kwi->keyword   = _keyword;
  kwi->isSymbol  = _isSymbol;
  kwi->region    = _region;
  kwi->ssShorter = _ssShorter;
};
KeywordInfo::KeywordInfo(KeywordInfo &ki)
{
  keyword = new SString(ki.keyword);
  region = ki.region;
  isSymbol = ki.isSymbol;
  ssShorter = ki.ssShorter;
};
KeywordInfo &KeywordInfo::operator=(KeywordInfo &ki)
{
  delete keyword;
  keyword = new SString(ki.keyword);
  region = ki.region;
  isSymbol = ki.isSymbol;
  ssShorter = ki.ssShorter;
  return *this;
};
KeywordInfo::~KeywordInfo()
{
  delete keyword;
};
*/

KeywordList::KeywordList()
{
  num = 0;
  matchCase = false;
  minKeywordLength = 0;
  kwList = nullptr;
  firstChar = new CharacterClass();
}

KeywordList::~KeywordList()
{
  for (int idx = 0; idx < num; idx++) {
    delete kwList[idx].keyword;
  }
  delete[] kwList;
  delete   firstChar;
}

int kwCompare(const void* e1, const void* e2)
{
  return ((KeywordInfo*)e1)->keyword->compareTo(*((KeywordInfo*)e2)->keyword);
}

int kwCompareI(const void* e1, const void* e2)
{
  return ((KeywordInfo*)e1)->keyword->compareToIgnoreCase(*((KeywordInfo*)e2)->keyword);
}

void KeywordList::sortList()
{
  if (num < 2) {
    return;
  }

  if (matchCase) {
    qsort((void*)kwList, num, sizeof(KeywordInfo), &kwCompare);
  } else {
    qsort((void*)kwList, num, sizeof(KeywordInfo), &kwCompareI);
  }
}

/* Searches previous elements num with same partial name
   fe:
   3: getParameterName  0
   2: getParameter      0
   1: getParam          0
   0: getPar            -1
*/
void KeywordList::substrIndex()
{
  for (int i = num - 1; i > 0; i--)
    for (int ii = i - 1; ii != 0; ii--) {
      if ((*kwList[ii].keyword)[0] != (*kwList[i].keyword)[0]) {
        break;
      }
      if (kwList[ii].keyword->length() < kwList[i].keyword->length() &&
          DString(kwList[i].keyword, 0, kwList[ii].keyword->length()) == *kwList[ii].keyword) {
        kwList[i].ssShorter = ii;
        break;
      }
    }
}
