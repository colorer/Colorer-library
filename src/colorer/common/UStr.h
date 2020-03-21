#ifndef COLORER_COMMON_USTR_H_
#define COLORER_COMMON_USTR_H_

#include <xercesc/util/XMLString.hpp>
#include <colorer/Common.h>
#include <unicode/uniset.h>

class UStr
{
 public:
  [[nodiscard]] static UnicodeString to_unistr(int number);
  [[nodiscard]] static std::string to_stdstr(const UnicodeString* str);
  [[nodiscard]] static std::string to_stdstr(const XMLCh* str);
  [[nodiscard]] static std::unique_ptr<XMLCh[]> to_xmlch(const UnicodeString* str);

  static bool isLowerCase(UChar c);
  static bool isUpperCase(UChar c);
  static bool isLetter(UChar c);
  static bool isLetterOrDigit(UChar c);
  static bool isDigit(UChar c);
  static bool isWhitespace(UChar c);

  static UChar toLowerCase(UChar c);
  static UChar toUpperCase(UChar c);

  static icu::UnicodeSet* createCharClass(const UnicodeString &ccs, int pos, int* retPos);
  /** \\x{2028} \\x23 \\c  - into wchar
    @param str String to parse Escape sequence.
    @param pos Position, where sequence starts.
    @param retPos Returns here string position after parsed
           character escape.
    @return If bad sequence, returns BAD_WCHAR.
            Else converts character escape and returns it's unicode value.
*/
  static UChar getEscapedChar(const UnicodeString &str, int pos, int &retPos);
  static int getHex(UChar c);
  static int getHexNumber(const UnicodeString* pstr);
  static int getNumber(const UnicodeString* pstr);
  static bool getNumber(const UnicodeString* pstr, double* res);
  static bool getNumber(const UnicodeString* pstr, int* res);
  /** For '{name}'  returns 'name'
    Removes brackets and returns new dynamic string,
    based on passed string.
    Returns null if parse error occurs.
    @note Returned dynamic string implies, than
          base passed string remains valid until
          accessing it.
*/
  static UnicodeString* getCurlyContent(const UnicodeString &str, int pos);
};

#endif  //COLORER_COMMON_USTR_H_
