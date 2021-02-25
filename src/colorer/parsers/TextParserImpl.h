#ifndef _COLORER_TEXTPARSERIMPL_H_
#define _COLORER_TEXTPARSERIMPL_H_

#include <colorer/TextParser.h>
#include <colorer/parsers/TextParserHelpers.h>

#define MAX_RECURSION_LEVEL 100

/**
 * Implementation of TextParser interface.
 * This is the base Colorer syntax parser, which
 * works with parsed internal HRC structure and colorisez
 * text in a target editor system.
 * @ingroup colorer_parsers
 */
class TextParser::Impl
{
 public:
  Impl();
  ~Impl();

  void setFileType(FileType* type);
  void setLineSource(LineSource* lh);
  void setRegionHandler(RegionHandler* rh);
  int parse(int from, int num, TextParseMode mode);
  void breakParse();
  void clearCache();
  void setMaxBlockSize(int max_block_size);

 private:
  UnicodeString* str = nullptr;
  int stackLevel = 0;
  int gy = 0;
  int gx = 0;
  int gy2 = 0;
  int len = -1;
  int clearLine = -1;
  int endLine = 0;
  int schemeStart = -1;
  SchemeImpl* baseScheme = nullptr;

  bool breakParsing = false;
  bool invisibleSchemesFilled = false;
  bool updateCache = false;
  const Region* picked = nullptr;

  ParseCache* cache = nullptr;
  ParseCache* parent = nullptr;
  ParseCache* forward = nullptr;

  SMatches matchend = {};
  VTList* vtlist = nullptr;

  LineSource* lineSource = nullptr;
  RegionHandler* regionHandler = nullptr;

  // maximum block size of regexp in string line
  int maxBlockSize = 1000;

  void fillInvisibleSchemes(ParseCache* cache);
  void addRegion(int lno, int sx, int ex, const Region* region);
  void enterScheme(int lno, int sx, int ex, const Region* region);
  void leaveScheme(int lno, int sx, int ex, const Region* region);
  void enterScheme(int lno, SMatches* match, const SchemeNode* schemeNode);
  void leaveScheme(int lno, SMatches* match, const SchemeNode* schemeNode);

  int searchKW(const SchemeNode* node, int no, int lowLen, int hiLen);
  int searchRE(SchemeImpl* cscheme, int no, int lowLen, int hiLen);
  bool colorize(CRegExp* root_end_re, bool lowContentPriority);
};

#endif
