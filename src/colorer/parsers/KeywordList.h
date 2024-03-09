#ifndef COLORER_KEYWORDLIST_H
#define COLORER_KEYWORDLIST_H

#include <unicode/uniset.h>
#include <climits>
#include "colorer/Common.h"
#include "colorer/Region.h"

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
  int indexOfShorter = -1;
};

/** List of keywords.
    @ingroup colorer_parsers
*/
class KeywordList
{
 public:
  bool matchCase = false;
  int count = 0;
  int minKeywordLength = INT_MAX;
  std::unique_ptr<icu::UnicodeSet> firstChar;
  KeywordInfo* kwList = nullptr;
  explicit KeywordList(size_t list_size);
  ~KeywordList();
  void sortList();
  void substrIndex();
};

#endif  //COLORER_KEYWORDLIST_H
