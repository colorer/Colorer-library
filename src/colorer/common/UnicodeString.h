#ifndef COLORER_UNICODESTRING_H
#define COLORER_UNICODESTRING_H

#include <unicode/unistr.h>
#include <memory>

typedef icu_63::UnicodeString UnicodeString;
typedef std::unique_ptr<UnicodeString> uUnicodeString;

class String;

class UStr
{
 public:
  static UnicodeString to_unistr(const String* str);
  static UnicodeString to_unistr(int number);
};

#endif  // COLORER_UNICODESTRING_H
