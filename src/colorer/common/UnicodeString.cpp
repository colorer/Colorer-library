#include <colorer/common/UnicodeString.h>
#include <colorer/unicode/String.h>

UnicodeString UStr::to_unistr(const String* str)
{
  return UnicodeString(str->getWChars());
}

UnicodeString UStr::to_unistr(int number)
{
  return UnicodeString(std::to_string(number).c_str());
}
