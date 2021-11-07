#ifndef COLORER_USTR_H
#define COLORER_USTR_H

#include <unicode/uniset.h>
#include <filesystem>
#include <xercesc/util/XMLString.hpp>
#include "colorer/Common.h"

class UStr
{
 public:
  [[nodiscard]] static UnicodeString to_unistr(int number);
  [[nodiscard]] static std::string to_stdstr(const UnicodeString* str);
  [[nodiscard]] static std::string to_stdstr(const uUnicodeString& str);
  [[nodiscard]] static std::string to_stdstr(const XMLCh* str);
  [[nodiscard]] static std::unique_ptr<XMLCh[]> to_xmlch(const UnicodeString* str);
  [[nodiscard]] static std::filesystem::path to_filepath(const uUnicodeString& str);
#ifdef _WINDOWS
  [[nodiscard]] static std::wstring to_stdwstr(const UnicodeString* str);
  [[nodiscard]] static std::wstring to_stdwstr(const uUnicodeString& str);
#endif

  inline static bool isEmpty(const XMLCh* string)
  {
    return *string == '\0';
  }
  static bool isLowerCase(UChar c);
  static bool isUpperCase(UChar c);
  static bool isLetter(UChar c);
  static bool isLetterOrDigit(UChar c);
  static bool isDigit(UChar c);
  static bool isWhitespace(UChar c);

  static UChar toLowerCase(UChar c);
  static UChar toUpperCase(UChar c);

  static std::unique_ptr<icu::UnicodeSet> createCharClass(const UnicodeString& ccs, int pos,
                                                          int* retPos, bool ignore_case);
  /** \\x{2028} \\x23 \\c  - into wchar
    @param str String to parse Escape sequence.
    @param pos Position, where sequence starts.
    @param retPos Returns here string position after parsed
           character escape.
    @return If bad sequence, returns BAD_WCHAR.
            Else converts character escape and returns it's unicode value.
*/
  static UChar getEscapedChar(const UnicodeString& str, int pos, int& retPos);
  static int getHex(UChar c);
  static int getHexNumber(const UnicodeString* pstr);
  static int getNumber(const UnicodeString* pstr);

  /** For '{name}'  returns 'name'
    Removes brackets and returns new dynamic string,
    based on passed string.
    Returns null if parse error occurs.
    @note Returned dynamic string implies, than
          base passed string remains valid until
          accessing it.
*/
  static uUnicodeString getCurlyContent(const UnicodeString& str, int pos);

  static bool HexToUInt(const UnicodeString& str_hex, unsigned int* result);
};

#endif  // COLORER_USTR_H
