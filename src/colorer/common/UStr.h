#ifndef COLORER_COMMON_USTR_H_
#define COLORER_COMMON_USTR_H_

#include <xercesc/util/XMLString.hpp>
#include <colorer/Common.h>

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
};

#endif  //COLORER_COMMON_USTR_H_
