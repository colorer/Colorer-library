#include <colorer/common/UnicodeString.h>
#include <colorer/unicode/SString.h>
#include <colorer/unicode/String.h>

UnicodeString UStr::to_unistr(const String* str)
{
  return UnicodeString(str->getWChars());
}

UnicodeString UStr::to_unistr(int number)
{
  return UnicodeString(std::to_string(number).c_str());
}

SString UStr::to_string(const UnicodeString* str)
{
  if (!str) return SString();
  auto len = str->length();
  std::unique_ptr<UChar[]> out_s(new UChar[len + 1]);
  str->extract(0, len, out_s.get());
  out_s[len] = 0;
  return SString(reinterpret_cast<const wchar*>(out_s.get()));
}
