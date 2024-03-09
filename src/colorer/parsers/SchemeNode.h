#ifndef _COLORER_SCHEMENODE_H_
#define _COLORER_SCHEMENODE_H_

#include "colorer/Common.h"
#include "colorer/Region.h"
#include "colorer/cregexp/cregexp.h"
#include "colorer/parsers/KeywordList.h"
#include "colorer/parsers/VirtualEntry.h"
#include <vector>

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
  enum class SchemeNodeType { SNT_RE, SNT_BLOCK, SNT_KEYWORDS, SNT_INHERIT };
  static constexpr std::string_view schemeNodeTypeNames[] = {"RE", "BLOCK", "KEYWORDS", "INHERIT"};

  SchemeNodeType type;

  uUnicodeString schemeName = nullptr;
  SchemeImpl* scheme = nullptr;

  std::unique_ptr<VirtualEntryVector> virtualEntryVector;
  std::unique_ptr<KeywordList> kwList;
  std::unique_ptr<icu::UnicodeSet> worddiv;

  const Region* region = nullptr;
  const Region* regions[REGIONS_NUM] = {};
  const Region* regionsn[NAMED_REGIONS_NUM] = {};
  const Region* regione[REGIONS_NUM] = {};
  const Region* regionen[NAMED_REGIONS_NUM] = {};
  std::unique_ptr<CRegExp> start;
  std::unique_ptr<CRegExp> end;
  bool innerRegion = false;
  bool lowPriority = false;
  bool lowContentPriority = false;

  explicit SchemeNode(SchemeNodeType type);
  ~SchemeNode();
};

#endif  //_COLORER_SCHEMENODE_H_
