#include <colorer/common/UStr.h>

UnicodeString UStr::to_unistr(const int number)
{
  return UnicodeString(std::to_string(number).c_str());
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
  if (str) {
    str->toUTF8String(out_str);
  }
  return out_str;
}

std::wstring UStr::to_stdwstr(const UnicodeString* str)
{
  std::wstring out_string;
  // wchar_t and UChar are the same size
  if (str) {
    auto len = str->length();
    std::unique_ptr<wchar_t[]> out_s(new wchar_t[len + 1]);
    str->extract(0, len, out_s.get());
    out_s[len] = 0;

    out_string.assign(out_s.get());
  }
  return out_string;
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

icu::UnicodeSet* UStr::createCharClass(const UnicodeString& ccs, int pos, int* retPos, bool ignore_case)
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
    if (ccs[pos] == '{') {
      UnicodeString* categ = getCurlyContent(ccs, pos);
      if (categ == nullptr) {
        delete cc;
        return nullptr;
      }
      /*if (*categ == "ALL") cc->add(icu::UnicodeSet::MIN_VALUE, icu::UnicodeSet::MAX_VALUE);
      else if (*categ == "ASSIGNED") cc->addCategory("");
      else if (*categ == "UNASSIGNED") {
        cc_temp.clear();
        cc_temp.addCategory("");
        cc->fill();
        cc->clearClass(cc_temp);
      } else */
      if (categ->length())
        cc->addAll(icu::UnicodeSet("\\p{" + *categ + "}", ec));
      pos += categ->length() + 1;
      delete categ;
      prev_char = BAD_WCHAR;
      continue;
    }
    if (ccs[pos] == '\\' && pos + 1 < ccs.length()) {
      int retEnd;
      prev_char = BAD_WCHAR;
      switch (ccs[pos + 1]) {
        case 'd':
          cc->addAll(icu::UnicodeSet("[:Nd:]", ec));
          break;
        case 'D':
          cc->addAll(icu::UnicodeSet(icu::UnicodeSet::MIN_VALUE, icu::UnicodeSet::MAX_VALUE).removeAll(icu::UnicodeSet("\\p{Nd}", ec)));
          break;
        case 'w':
          cc->addAll(icu::UnicodeSet("[:L:]", ec)).addAll(icu::UnicodeSet("\\p{Nd}", ec)).add("_");
          break;
        case 'W':
          cc->addAll(icu::UnicodeSet(icu::UnicodeSet::MIN_VALUE, icu::UnicodeSet::MAX_VALUE)
                         .removeAll(icu::UnicodeSet("\\p{Nd}", ec))
                         .removeAll(icu::UnicodeSet("\\p{L}", ec)))
              .remove("_");
          break;
        case 's':
          cc->addAll(icu::UnicodeSet("[:Z:]", ec)).addAll("\t\n\r\f");
          break;
        case 'S':
          cc->addAll(icu::UnicodeSet(icu::UnicodeSet::MIN_VALUE, icu::UnicodeSet::MAX_VALUE).removeAll(icu::UnicodeSet("[:Z:]", ec)))
              .removeAll("\t\n\r\f");
          break;
        case 'l':
          cc->addAll(icu::UnicodeSet("[:Ll:]", ec));
          if (ignore_case)
            cc->addAll(icu::UnicodeSet("[:Lu:]", ec));
          break;
        case 'u':
          cc->addAll(icu::UnicodeSet("[:Lu:]", ec));
          if (ignore_case)
            cc->addAll(icu::UnicodeSet("[:Ll:]", ec));
          break;
        default:
          prev_char = getEscapedChar(ccs, pos, retEnd);
          if (prev_char == BAD_WCHAR)
            break;
          cc->add(prev_char);
          if (ignore_case) {
            cc->add(u_tolower(prev_char));
            cc->add(u_toupper(prev_char));
            cc->add(u_totitle(prev_char));
          }
          pos = retEnd - 1;
          break;
      }
      pos++;
      continue;
    }
    // substract -[class]
    if (pos + 1 < ccs.length() && ccs[pos] == '-' && ccs[pos + 1] == '[') {
      int retEnd;
      icu::UnicodeSet* scc = createCharClass(ccs, pos + 1, &retEnd, false);
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
      int retEnd;
      icu::UnicodeSet* scc = createCharClass(ccs, pos + 2, &retEnd, false);
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
      int retEnd;
      icu::UnicodeSet* scc = createCharClass(ccs, pos, &retEnd, ignore_case);
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
    if (ignore_case) {
      cc->add(u_tolower(prev_char));
      cc->add(u_toupper(prev_char));
      cc->add(u_totitle(prev_char));
    }
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
        UnicodeString* val = getCurlyContent(str, pos + 2);
        if (val == nullptr)
          return BAD_WCHAR;
        int tmp = getHexNumber(val);
        int val_len = val->length();
        delete val;
        if (tmp < 0 || tmp > 0xFFFF)
          return BAD_WCHAR;
        retPos += val_len + 2;
        return tmp;
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

UnicodeString* UStr::getCurlyContent(const UnicodeString& str, int pos)
{
  if (str[pos] != '{')
    return nullptr;
  int lpos;
  for (lpos = pos + 1; lpos < str.length(); lpos++) {
    if (str[lpos] == '}')
      break;
    if (!u_isgraph(str[lpos]))
      return nullptr;
  }
  if (lpos == str.length())
    return nullptr;
  return new UnicodeString(str, pos + 1, lpos - pos - 1);
}

int UStr::getNumber(const UnicodeString* pstr)
{
  int r = 1, num = 0;
  if (pstr == nullptr)
    return -1;
  for (int i = pstr->length() - 1; i >= 0; i--) {
    if ((*pstr)[i] > '9' || (*pstr)[i] < '0')
      return -1;
    num += ((*pstr)[i] - 0x30) * r;
    r *= 10;
  }
  return num;
}

bool UStr::HexToUInt(const UnicodeString& str_hex, unsigned int* result)
{
  UnicodeString s;
  if (str_hex[0] == '#')
    s = UnicodeString(str_hex, 1);
  else
    s = str_hex;

  try {
    *result = std::stoul(UStr::to_stdstr(&s), nullptr, 16);
    return true;
  } catch (std::exception& e) {
    spdlog::error("Can`t convert {0} to int. {1}", str_hex, e.what());
    return false;
  }
}
