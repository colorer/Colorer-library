
// really, you need to rewrite SMatches structure, if you
// porting this interface into other languages
#include<cregexp/cregexp.h>
//

#include<windows.h>
//
typedef CRegExp *PRegExp;
typedef SMatches *PMatches;

typedef struct SRegExpClass
{
            // Size of this struct
  int     sz;
            // Creates RegExp object, returns handle
            // you can create multiple regexp objects
  PRegExp (WINAPI *reCreate)();
            // Creates RegExp Object and compile expression. returns handle.
  PRegExp (WINAPI *reCreateCompile)(char *text, int codepage);
            // unicode pattern
  PRegExp (WINAPI *reCreateCompileW)(wchar *text);
            // isok?
  BOOL    (WINAPI *reIsOk)(PRegExp re);
  EError  (WINAPI *reGetError)(PRegExp re);
            // Destroys RegExp Object by handle
  BOOL    (WINAPI *reDestroy)(PRegExp re);

            // Disables movement throw the parsed line.
            // if Moves = true  -  regexp tested only at start position
            // else code tries to match regexp on all positions of string.
            // Of course, second variant is much slower (it parses regexp
            // to each position on the string)
            // It is false by default.
  BOOL    (WINAPI *reSetPositionMoves)(PRegExp re, BOOL positionMoves);
            // Compile Expression expr
  BOOL    (WINAPI *reSetRE)(PRegExp re, char *expr, int codepage);
  BOOL    (WINAPI *reSetREW)(PRegExp re, wchar *expr);
            // Parse compiled expression on string str, write result brackets
            // into mtch, and return true if success.
  BOOL    (WINAPI *reParse)(PRegExp re, char *str, int codepage, PMatches mtch);
  BOOL    (WINAPI *reParseW)(PRegExp re, wchar *str, PMatches mtch);
            // As previouse, but you can directly set start of line(Sol),
            // end of line (Eol) and moving
            // if moves == -1, then used value, setted in reSetNoMoves.
  BOOL    (WINAPI *reParseParam)(PRegExp re, char *str, int codepage, int pos, int eol, PMatches mtch, int SoS, BOOL moves);
  BOOL    (WINAPI *reParseParamW)(PRegExp re, wchar *str, int pos, int eol, PMatches mtch, int SoS, BOOL moves);
            //
  int     (WINAPI *ucsGetCodepageIndex)(char *cp);
  char*   (WINAPI *ucsGetCodepageName)(int cp);
} *PRegExpClass;

            // The "GetRegExpProc" exported function
            // returns pointers to all library functions
typedef BOOL (WINAPI *PRegExpProc)(PRegExpClass);

//  Regular Expressions syntax description You'll find in colorer's docs.
