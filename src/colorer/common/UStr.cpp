#include <colorer/common/UStr.h>
#include <colorer/unicode/SString.h>
#include <colorer/unicode/String.h>
#include <unicode/ucnv.h>
#include <unicode/uchar.h>

UnicodeString UStr::to_unistr(const String* str)
{
  return str ? UnicodeString(str->getWChars()) : UnicodeString();
}

UnicodeString UStr::to_unistr(const int number)
{
  return UnicodeString(std::to_string(number).c_str());
}

SString UStr::to_string(const UnicodeString* str)
{
  if (!str)
    return SString();
  auto len = str->length();
  std::unique_ptr<UChar[]> out_s(new UChar[len + 1]);
  str->extract(0, len, out_s.get());
  out_s[len] = 0;
  return SString(reinterpret_cast<const wchar*>(out_s.get()));
}

std::unique_ptr<XMLCh[]> UStr::to_xmlch(const UnicodeString* str)
{
  // XMLCh and UChar are the same size
  if (str) {
    auto len = str->length();
    std::unique_ptr<XMLCh[]> out_s(new XMLCh[len + 1]);
    str->extract(0, len, out_s.get());
    out_s[len] = 0;

    return out_s;
  } else {
    return nullptr;
  }
}

std::string UStr::to_stdstr(const UnicodeString* str)
{
  std::string out_str;
  str->toUTF8String(out_str);
  return out_str;
}

std::string UStr::to_stdstr(const XMLCh* str)
{
  std::string _string = std::string(xercesc::XMLString::transcode(str));
  return _string;
}

bool UStr::isLowerCase(UChar c)
{
  return u_islower(c);
}

bool UStr::isUpperCase(UChar c)
{
  return u_isupper(c);
}

bool UStr::isLetter(UChar c)
{
  return u_isalpha(c);
}

bool UStr::isLetterOrDigit(UChar c)
{
  return u_isdigit(c) || u_isalpha(c);
}

bool UStr::isDigit(UChar c)
{
  return u_isdigit(c);
}

bool UStr::isWhitespace(UChar c)
{
  return u_isspace(c);
}

UChar UStr::toLowerCase(UChar c)
{
  return u_tolower(c);
}

UChar UStr::toUpperCase(UChar c)
{
  return u_toupper(c);
}
