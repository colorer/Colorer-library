#ifndef COLORER_UNICODESTRING_H
#define COLORER_UNICODESTRING_H

#include <unicode/unistr.h>
#include <memory>
#include <xercesc/util/XMLString.hpp>

typedef icu_63::UnicodeString UnicodeString;
typedef std::unique_ptr<UnicodeString> uUnicodeString;

class String;
class SString;

class UStr
{
 public:
  static UnicodeString to_unistr(const String* str);
  static UnicodeString to_unistr(int number);
  static SString to_string(const UnicodeString* str);
  static std::unique_ptr<XMLCh[]> to_xmlch(const UnicodeString* str);
  static std::string to_stdstr(const UnicodeString* str);
};

namespace std {
// Specializations for unordered containers

template <>
struct hash<icu_63::UnicodeString>
{
  size_t operator()(const icu_63::UnicodeString& value) const
  {
    return static_cast<std::size_t>(value.hashCode());
  }
};

template <>
struct equal_to<icu_63::UnicodeString>
{
  bool operator()(const icu_63::UnicodeString& u1, const icu_63::UnicodeString& u2) const
  {
    return u1.compare(u2) == 0;
  }
};

}  // namespace std
#endif  // COLORER_UNICODESTRING_H
