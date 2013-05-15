
#include"iregexp.h"

// just a ports...
PRegExp WINAPI reCreate()
{
  return new CRegExp();
};

PRegExp WINAPI reCreateCompile(char *text, int cp)
{
  return new CRegExp(&CDString(text, 0, -1, cp));
};

PRegExp WINAPI reCreateCompileW(wchar *text)
{
  return new CRegExp(&CDString(text));
};

BOOL WINAPI reIsOk(PRegExp re)
{
  return re->isOk();
};
EError WINAPI reGetError(PRegExp re)
{
  return re->getError();
};

BOOL WINAPI reDestroy(PRegExp re)
{
  delete re;
  return re?TRUE:FALSE;
};

BOOL WINAPI reSetPositionMoves(PRegExp re, BOOL Moves)
{
  if (!re) return FALSE;
  return re->setPositionMoves(Moves != 0);
};
BOOL WINAPI reSetRE(PRegExp re, char *expr, int cp)
{
  if (!re) return FALSE;
  return re->setRE(&CDString(expr, 0, -1, cp));
};
BOOL WINAPI reSetREW(PRegExp re, wchar *expr)
{
  if (!re) return FALSE;
  return re->setRE(&CDString(expr));
};

BOOL WINAPI reParse(PRegExp re, char *str, int cp, PMatches mtch)
{
  if (!re) return FALSE;
  return re->parse(&CDString(str, 0, -1, cp), mtch);
};
BOOL WINAPI reParseW(PRegExp re, wchar *str, PMatches mtch)
{
  if (!re) return FALSE;
  return re->parse(&CDString(str), mtch);
};

BOOL WINAPI reParseParam(PRegExp re, char *str, int cp, int pos, int eol, PMatches mtch, int SoS, BOOL moves)
{
  if (!re) return FALSE;
  return re->parse(&CDString(str, 0, -1, cp), pos, eol, mtch, SoS, moves);
};
BOOL WINAPI reParseParamW(PRegExp re, wchar *str, int pos, int eol, PMatches mtch, int SoS, BOOL moves)
{
  if (!re) return FALSE;
  return re->parse(&CDString(str), pos, eol, mtch, SoS, moves);
};

int WINAPI ucsGetCodepageIndex(char *cp){
  return CString::getCodepageIndex(cp);
};
char * WINAPI ucsGetCodepageName(int cp){
  return CString::getCodepageName(cp);
};

// structure with function pointers to return to the target program...
SRegExpClass sre = {
  sizeof(sre),
  reCreate,
  reCreateCompile,
  reCreateCompileW,
  reIsOk,
  reGetError,
  reDestroy,
  reSetPositionMoves,
  reSetRE,
  reSetREW,
  reParse,
  reParseW,
  reParseParam,
  reParseParamW,
  ucsGetCodepageIndex,
  ucsGetCodepageName
};

extern "C" BOOL WINAPI _export GetRegExpProc(PRegExpClass pre)
{
  if (!pre) return FALSE;
  int sz = pre->sz;
  if (sre.sz < sz) sz = sre.sz;
  MoveMemory(pre, &sre, sz);
  pre->sz = sz;
  return TRUE;
};