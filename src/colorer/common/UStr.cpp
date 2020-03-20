#include <colorer/common/UStr.h>
#include <colorer/unicode/SString.h>
#include <colorer/unicode/String.h>
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

icu::UnicodeSet* UStr::createCharClass(const UnicodeString& ccs, unsigned int pos, unsigned int* retPos)
{
  if (ccs[pos] != '[')
    return nullptr;

  icu::UnicodeSet* cc = new icu::UnicodeSet();
  icu::UnicodeSet cc_temp;
  bool inverse = false;
  UChar prev_char = BAD_WCHAR;
  UErrorCode ec = U_ZERO_ERROR;

  pos++;
  if (ccs[pos] == '^') {
    inverse = true;
    pos++;
  }

  for (; pos < ccs.length(); pos++) {
    if (ccs[pos] == ']') {
      if (retPos != nullptr)
        *retPos = pos;
      if (inverse) {
        cc->complement();
      }
      return cc;
    }
    /*if (ccs[pos] == '{') {
      String* categ = UnicodeTools::getCurlyContent(ccs, pos);
      if (categ == nullptr) {
        delete cc;
        return nullptr;
      }
      if (*categ == CString("ALL")) cc->fill();
      else if (*categ == CString("ASSIGNED")) cc->addCategory("");
      else if (*categ == CString("UNASSIGNED")) {
        cc_temp.clear();
        cc_temp.addCategory("");
        cc->fill();
        cc->clearClass(cc_temp);
      } else if (categ->length()) cc->addCategory(*categ);
      pos += categ->length() + 1;
      delete categ;
      prev_char = BAD_WCHAR;
      continue;
    }*/
    if (ccs[pos] == '\\' && pos + 1 < ccs.length()) {
      int retEnd;
      prev_char = BAD_WCHAR;
      switch (ccs[pos + 1]) {
        case 'd':
          cc->addAll(icu::UnicodeSet("[:Nd:]", ec));
          break;
        case 'D':
          cc->addAll(icu::UnicodeSet("[:^Nd:]", ec));
          break;
        case 'w':
          cc->addAll(icu::UnicodeSet("[:L:][:Nd:]", ec)).add("_");
          break;
        case 'W':
          cc->addAll(icu::UnicodeSet(icu::UnicodeSet::MIN_VALUE, icu::UnicodeSet::MAX_VALUE)
                         .removeAll(icu::UnicodeSet("\\p{Nd}", ec))
                         .removeAll(icu::UnicodeSet("\\p{L}", ec)))
              .remove("_");
          break;
        case 's':
          cc->addAll(icu::UnicodeSet("[:Z:]\t\n\r\f", ec));
          break;
        case 'S':
          cc->addAll(icu::UnicodeSet("[^:Z:][^\t\n\r\f]", ec));
          break;
        case 'l':
          cc->addAll(icu::UnicodeSet("[:Ll:]", ec));
          break;
        case 'u':
          cc->addAll(icu::UnicodeSet("[:Lu:]", ec));
          break;
        default:
          prev_char = getEscapedChar(ccs, pos, retEnd);
          if (prev_char == BAD_WCHAR)
            break;
          cc->add(prev_char);
          pos = retEnd - 1;
          break;
      }
      pos++;
      continue;
    }
    // substract -[class]
    if (pos + 1 < ccs.length() && ccs[pos] == '-' && ccs[pos + 1] == '[') {
      unsigned int retEnd;
      icu::UnicodeSet* scc = createCharClass(ccs, pos + 1, &retEnd);
      if (retEnd == ccs.length()) {
        delete cc;
        return nullptr;
      }
      if (scc == nullptr) {
        delete cc;
        return nullptr;
      }
      cc->removeAll(*scc);
      delete scc;
      pos = retEnd;
      prev_char = BAD_WCHAR;
      continue;
    }
    // intersect &&[class]
    if (pos + 2 < ccs.length() && ccs[pos] == '&' && ccs[pos + 1] == '&' && ccs[pos + 2] == '[') {
      unsigned int retEnd;
      icu::UnicodeSet* scc = createCharClass(ccs, pos + 2, &retEnd);
      if (retEnd == ccs.length()) {
        delete cc;
        return nullptr;
      }
      if (scc == nullptr) {
        delete cc;
        return nullptr;
      }
      cc->retainAll(*scc);
      delete scc;
      pos = retEnd;
      prev_char = BAD_WCHAR;
      continue;
    }
    // add [class]
    if (ccs[pos] == '[') {
      unsigned int retEnd;
      icu::UnicodeSet* scc = createCharClass(ccs, pos, &retEnd);
      if (scc == nullptr) {
        delete cc;
        return nullptr;
      }
      cc->addAll(*scc);
      delete scc;
      pos = retEnd;
      prev_char = BAD_WCHAR;
      continue;
    }
    if (ccs[pos] == '-' && prev_char != BAD_WCHAR && pos + 1 < ccs.length() && ccs[pos + 1] != ']') {
      int retEnd;
      UChar nextc = getEscapedChar(ccs, pos + 1, retEnd);
      if (nextc == BAD_WCHAR)
        break;
      cc->add(prev_char, nextc);
      pos = retEnd;
      continue;
    }
    cc->add(ccs[pos]);
    prev_char = ccs[pos];
  }
  delete cc;

  return nullptr;
}

UChar UStr::getEscapedChar(const UnicodeString& str, int pos, int& retPos)
{
  retPos = pos;
  if (str[pos] == '\\') {
    retPos++;
    if (str[pos + 1] == 'x') {
      if (str[pos + 2] == '{') {
        /* String* val = getCurlyContent(str, pos + 2);
         if (val == nullptr) return BAD_WCHAR;
         int tmp = getHexNumber(val);
         int val_len = val->length();
         delete val;
         if (tmp < 0 || tmp > 0xFFFF) return BAD_WCHAR;
         retPos += val_len + 2;
         return tmp;*/
      } else {
        UnicodeString dtmp = UnicodeString(str, pos + 2, 2);
        int tmp = getHexNumber(&dtmp);
        if (str.length() <= pos + 2 || tmp == -1)
          return BAD_WCHAR;
        retPos += 2;
        return tmp;
      }
    }
    return str[pos + 1];
  }
  return str[pos];
}

int UStr::getHex(UChar c)
{
  c = toLowerCase(c);
  c -= '0';
  if (c >= 'a' - '0' && c <= 'f' - '0')
    c -= 0x27;
  else if (c > 9)
    return -1;
  return c;
}

int UStr::getHexNumber(const UnicodeString* pstr)
{
  int r = 0, num = 0;
  if (pstr == nullptr)
    return -1;
  for (int i = (*pstr).length() - 1; i >= 0; i--) {
    int d = getHex((*pstr)[i]);
    if (d == -1)
      return -1;
    num += d << r;
    r += 4;
  }
  return num;
}