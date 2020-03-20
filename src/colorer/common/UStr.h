#ifndef COLORER_COMMON_USTR_H_
#define COLORER_COMMON_USTR_H_

#include <xercesc/util/XMLString.hpp>
#include <colorer/Common.h>
#include <unicode/uniset.h>

class String;
class SString;

class UStr
{
 public:
  [[nodiscard]] static UnicodeString to_unistr(const String* str);
  [[nodiscard]] static UnicodeString to_unistr(int number);
  [[nodiscard]] static std::string to_stdstr(const UnicodeString* str);
  [[nodiscard]] static std::string to_stdstr(const XMLCh* str);
  [[nodiscard]] static SString to_string(const UnicodeString* str);
  [[nodiscard]] static std::unique_ptr<XMLCh[]> to_xmlch(const UnicodeString* str);

  static bool isLowerCase(UChar c);
  static bool isUpperCase(UChar c);
  static bool isLetter(UChar c);
  static bool isLetterOrDigit(UChar c);
  static bool isDigit(UChar c);
  static bool isWhitespace(UChar c);

  static UChar toLowerCase(UChar c);
  static UChar toUpperCase(UChar c);

  static icu::UnicodeSet* createCharClass(const UnicodeString &ccs, unsigned int pos, unsigned int* retPos);
  static UChar getEscapedChar(const UnicodeString &str, int pos, int &retPos);
  static int getHex(UChar c);
  static int getHexNumber(const UnicodeString* pstr);
};

#endif  //COLORER_COMMON_USTR_H_
