#ifndef _COLORER_KEYWORDLIST_H_
#define _COLORER_KEYWORDLIST_H_

#include "colorer/Common.h"
#include "colorer/Region.h"
#include <unicode/uniset.h>

/** Information about one parsed keyword.
    Contains keyword, symbol specifier, region reference
    and internal optimization field.
    @ingroup colorer_parsers
*/
struct KeywordInfo
{
  std::unique_ptr<const UnicodeString> keyword;
  const Region* region = nullptr;
  bool isSymbol = false;
  int ssShorter = -1;
};

/** List of keywords.
    @ingroup colorer_parsers
*/
class KeywordList
{
 public:
  int num = 0;
  bool matchCase = false;
  int minKeywordLength = 0;
  std::unique_ptr<icu::UnicodeSet> firstChar;
  KeywordInfo* kwList = nullptr;
  KeywordList();
  ~KeywordList();
  void sortList();
  void substrIndex();
};

#endif  //_COLORER_KEYWORDLIST_H_
