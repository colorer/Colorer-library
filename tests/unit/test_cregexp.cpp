#include <colorer/cregexp/cregexp.h>
#include <catch2/catch_amalgamated.hpp>
#include "colorer/ParserFactory.h"

namespace {

UnicodeString ustr(const char16_t* text)
{
  return UnicodeString(text);
}

bool parse_re(CRegExp& re, const UnicodeString& text, SMatches& match, bool moves = false, int pos = 0,
              int soscheme = 0)
{
  re.setPositionMoves(moves);
  return re.parse(&text, pos, text.length(), &match, soscheme);
}

void require_match(const char16_t* pattern, const char16_t* text, int s0, int e0, bool moves = false, int pos = 0)
{
  const auto pre = ustr(pattern);
  const auto str = ustr(text);
  CRegExp re(&pre);
  INFO("pattern: " << UStr::to_stdstr(&pre) << " text: " << UStr::to_stdstr(&str));
  REQUIRE(re.isOk());
  SMatches match;
  REQUIRE(parse_re(re, str, match, moves, pos));
  REQUIRE(match.s[0] == s0);
  REQUIRE(match.e[0] == e0);
}

void require_no_match(const char16_t* pattern, const char16_t* text, bool moves = false, int pos = 0)
{
  const auto pre = ustr(pattern);
  const auto str = ustr(text);
  CRegExp re(&pre);
  INFO("pattern: " << UStr::to_stdstr(&pre) << " text: " << UStr::to_stdstr(&str));
  REQUIRE(re.isOk());
  SMatches match;
  REQUIRE_FALSE(parse_re(re, str, match, moves, pos));
}

UnicodeString sequential_captures(int inner)
{
  UnicodeString pattern(u"/a");
  for (int i = 0; i < inner; i++) {
    pattern += UnicodeString(u"(x)");
  }
  pattern += UnicodeString(u"z/");
  return pattern;
}

UnicodeString named_captures(int count)
{
  UnicodeString pattern(u"/");
  for (int i = 0; i < count; i++) {
    pattern += UnicodeString(u"(?{n");
    pattern += UStr::to_unistr(i);
    pattern += UnicodeString(u"}x)");
  }
  pattern += UnicodeString(u"/");
  return pattern;
}

}  // namespace

TEST_CASE("CRegExp compilation", "[cregexp]")
{
  SECTION("empty object is not ready")
  {
    CRegExp re;
    REQUIRE_FALSE(re.isOk());
    REQUIRE(re.getError() == EError::EERROR);
  }

  SECTION("valid slash-delimited pattern")
  {
    const auto pattern = ustr(u"/abc/");
    CRegExp re(&pattern);
    REQUIRE(re.isOk());
    REQUIRE(re.getError() == EError::EOK);
  }

  SECTION("empty pattern // is valid")
  {
    require_match(u"//", u"", 0, 0);
    require_match(u"//", u"abc", 0, 0);
  }

  SECTION("setRE replaces previous pattern")
  {
    const auto first = ustr(u"/abc/");
    const auto second = ustr(u"/xyz/");
    const auto text = ustr(u"xyz");
    CRegExp re(&first);
    REQUIRE(re.setRE(&second));
    SMatches match;
    REQUIRE(re.parse(&text, &match));
    REQUIRE(match.s[0] == 0);
    REQUIRE(match.e[0] == 3);
  }

  SECTION("parse of invalid regexp fails")
  {
    CRegExp re;
    const auto text = ustr(u"abc");
    SMatches match;
    REQUIRE_FALSE(re.parse(&text, &match));
  }
}

TEST_CASE("CRegExp compilation errors", "[cregexp]")
{
  SECTION("missing delimiters")
  {
    const auto [pattern, error] = GENERATE(table<const char16_t*, EError>({
        {u"", EError::EERROR},
        {u"abc", EError::ESYNTAX},
        {u"/abc", EError::ESYNTAX},
        {u"abc/", EError::ESYNTAX},
    }));
    const auto pre = ustr(pattern);
    CRegExp re(&pre);
    INFO("pattern: " << UStr::to_stdstr(&pre));
    REQUIRE_FALSE(re.isOk());
    REQUIRE(re.getError() == error);
  }

  SECTION("malformed pattern")
  {
    const auto [pattern, error] = GENERATE(table<const char16_t*, EError>({
        {u"/(abc/", EError::EBRACKETS},
        {u"/abc)/", EError::EBRACKETS},
        {u"/[abc/", EError::EENUM},
        {u"/a{/", EError::EBRACKETS},
        {u"/+/", EError::EOP},
        {u"/{2}/", EError::EOP},
        {u"/a{5,2}/", EError::EOP},
        {u"/{5,2}/", EError::EOP},
        {u"/*/", EError::EOP},
        {u"/\\/", EError::ESYNTAX},
        {u"/\\y/", EError::ESYNTAX},
        {u"/\\p/", EError::ESYNTAX},
        {u"   ", EError::ESYNTAX},
    }));
    const auto pre = ustr(pattern);
    CRegExp re(&pre);
    INFO("pattern: " << UStr::to_stdstr(&pre));
    REQUIRE_FALSE(re.isOk());
    REQUIRE(re.getError() == error);
  }
}

TEST_CASE("CRegExp basic matching", "[cregexp]")
{
  const auto [pattern, text, should_match, s0, e0] = GENERATE(table<const char16_t*, const char16_t*, bool, int, int>({
      {u"/abc/", u"abc", true, 0, 3},
      {u"/abc/", u"ab", false, 0, 0},
      {u"/abc/", u"abcd", true, 0, 3},
      {u"/abc/", u"xabc", false, 0, 0},
      {u"/^abc/", u"abc", true, 0, 3},
      {u"/^abc/", u"xabc", false, 0, 0},
      {u"/abc$/", u"abc", true, 0, 3},
      {u"/abc$/", u"abcd", false, 0, 0},
      {u"/^abc$/", u"abc", true, 0, 3},
      {u"/a.c/", u"axc", true, 0, 3},
      {u"/\\d+/", u"42", true, 0, 2},
      {u"/\\D+/", u"ab", true, 0, 2},
      {u"/\\w+/", u"ab_1", true, 0, 4},
      {u"/\\s+/", u" \t", true, 0, 2},
      {u"/[abc]/", u"b", true, 0, 1},
      {u"/[^a]/", u"b", true, 0, 1},
      {u"/[^a]/", u"a", false, 0, 0},
      {u"/a*/", u"", true, 0, 0},
      {u"/a*/", u"b", true, 0, 0},
      {u"/a+/", u"", false, 0, 0},
      {u"/a+/", u"aaa", true, 0, 3},
      {u"/a?/", u"", true, 0, 0},
      {u"/a?/", u"a", true, 0, 1},
      {u"/a|b/", u"a", true, 0, 1},
      {u"/a|b/", u"b", true, 0, 1},
      {u"/ab|cd/", u"cd", true, 0, 2},
      {u"/a{2}/", u"aa", true, 0, 2},
      {u"/a{2}/", u"a", false, 0, 0},
      {u"/a{2,4}/", u"aaaaa", true, 0, 4},
      {u"/a{2,}/", u"aaa", true, 0, 3},
      {u"/\\t/", u"\t", true, 0, 1},
      {u"/\\n/", u"\n", true, 0, 1},
      {u"/\\x{41}/", u"A", true, 0, 1},
      {u"/a\\/b/", u"a/b", true, 0, 3},
  }));

  const auto pre = ustr(pattern);
  const auto str = ustr(text);
  CRegExp re(&pre);
  INFO("pattern: " << UStr::to_stdstr(&pre) << " text: " << UStr::to_stdstr(&str));
  REQUIRE(re.isOk());
  SMatches match;
  REQUIRE(re.parse(&str, &match) == should_match);
  if (should_match) {
    REQUIRE(match.s[0] == s0);
    REQUIRE(match.e[0] == e0);
  }
}

TEST_CASE("CRegExp positionMoves", "[cregexp]")
{
  SECTION("does not search forward by default")
  {
    require_no_match(u"/abc/", u"xabc");
  }

  SECTION("setPositionMoves searches inside the string")
  {
    require_match(u"/abc/", u"xabc", 1, 4, true);
  }

  SECTION("parse moves argument overrides the flag")
  {
    const auto pre = ustr(u"/abc/");
    const auto str = ustr(u"xxabc");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE(re.parse(&str, 0, str.length(), &match, 0, 1));
    REQUIRE(match.s[0] == 2);
    REQUIRE(match.e[0] == 5);
  }

  SECTION("parse range limits the search")
  {
    const auto pre = ustr(u"/abc/");
    const auto str = ustr(u"xxabcxx");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE(re.parse(&str, 2, 5, &match));
    REQUIRE(match.s[0] == 2);
    REQUIRE(match.e[0] == 5);
    REQUIRE_FALSE(re.parse(&str, 2, 4, &match));
  }
}

TEST_CASE("CRegExp flags", "[cregexp]")
{
  SECTION("ignore case")
  {
    require_match(u"/abc/i", u"ABC", 0, 3);
    require_match(u"/abc/i", u"AbC", 0, 3);
    require_no_match(u"/abc/", u"ABC");
    require_match(u"/[x]/i", u"X", 0, 1);
    require_match(u"/[x]/i", u"x", 0, 1);
    require_match(u"/[aX]/i", u"x", 0, 1);
    require_match(u"/[aX]/i", u"A", 0, 1);
    require_no_match(u"/[x]/", u"X");
  }

  SECTION("dot does not match newline unless /s")
  {
    require_no_match(u"/a.c/", u"a\nc");
    require_match(u"/a.c/s", u"a\nc", 0, 3);
  }

  SECTION("extended /x ignores whitespace in pattern")
  {
    require_match(u"/a b c/x", u"abc", 0, 3);
  }
}

TEST_CASE("CRegExp greedy and lazy quantifiers", "[cregexp]")
{
  require_match(u"/\".*\"/", u"\"a\"b\"", 0, 5);
  require_match(u"/\".*?\"/", u"\"a\"b\"", 0, 3);
  require_match(u"/a+?/", u"aaa", 0, 1);
  require_match(u"/a*?b/", u"aaab", 0, 4);
  require_match(u"/a??b/", u"b", 0, 1);
  require_match(u"/a??b/", u"ab", 0, 2);
}

TEST_CASE("CRegExp groups and backreferences", "[cregexp]")
{
  SECTION("numeric groups")
  {
    const auto pre = ustr(u"/(ab)c/");
    const auto str = ustr(u"abc");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE(re.parse(&str, &match));
    REQUIRE(match.cMatch == 2);
    REQUIRE(match.s[0] == 0);
    REQUIRE(match.e[0] == 3);
    REQUIRE(match.s[1] == 0);
    REQUIRE(match.e[1] == 2);
  }

  SECTION("two capturing groups")
  {
    const auto pre = ustr(u"/(a)(b)/");
    const auto str = ustr(u"ab");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE(re.parse(&str, &match));
    REQUIRE(match.cMatch == 3);
    REQUIRE(match.s[1] == 0);
    REQUIRE(match.e[1] == 1);
    REQUIRE(match.s[2] == 1);
    REQUIRE(match.e[2] == 2);
  }

  SECTION("a failed search offset does not keep captures")
  {
    const auto pre = ustr(u"/(a)x|b/");
    const auto str = ustr(u"ab");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE(parse_re(re, str, match, true));
    REQUIRE(match.s[0] == 1);
    REQUIRE(match.e[0] == 2);
    REQUIRE(match.s[1] == -1);
    REQUIRE(match.e[1] == -1);
  }

  SECTION("numeric groups beyond the slot limit stay non-capturing")
  {
    const auto ok_pattern = sequential_captures(MATCHES_NUM - 1);
    UnicodeString ok_text(u"a");
    for (int i = 0; i < MATCHES_NUM - 1; i++) {
      ok_text += UnicodeString(u"x");
    }
    ok_text += UnicodeString(u"z");
    CRegExp ok_re(&ok_pattern);
    REQUIRE(ok_re.isOk());
    SMatches ok_match;
    REQUIRE(ok_re.parse(&ok_text, &ok_match));
    REQUIRE(ok_match.cMatch == MATCHES_NUM);
    REQUIRE(ok_match.s[0] == 0);
    REQUIRE(ok_match.e[0] == ok_text.length());

    const auto overflow = sequential_captures(MATCHES_NUM);
    UnicodeString overflow_text(u"a");
    for (int i = 0; i < MATCHES_NUM; i++) {
      overflow_text += UnicodeString(u"x");
    }
    overflow_text += UnicodeString(u"z");
    CRegExp overflow_re(&overflow);
    REQUIRE(overflow_re.isOk());
    SMatches overflow_match;
    REQUIRE(overflow_re.parse(&overflow_text, &overflow_match));
    REQUIRE(overflow_match.cMatch == MATCHES_NUM);
    REQUIRE(overflow_match.s[0] == 0);
    REQUIRE(overflow_match.e[0] == overflow_text.length());
  }

  SECTION("named groups beyond the slot limit stay non-capturing")
  {
    const auto ok_pattern = named_captures(NAMED_MATCHES_NUM);
    CRegExp ok_re(&ok_pattern);
    REQUIRE(ok_re.isOk());

    const auto overflow = named_captures(NAMED_MATCHES_NUM + 1);
    CRegExp overflow_re(&overflow);
    REQUIRE(overflow_re.isOk());
    const auto last_name_n = UStr::to_unistr(NAMED_MATCHES_NUM);
    UnicodeString last_name(u"n");
    last_name += last_name_n;
    REQUIRE(overflow_re.getBracketNo(&last_name) == -1);
    const auto first_name = ustr(u"n0");
    REQUIRE(overflow_re.getBracketNo(&first_name) == 0);
  }

  SECTION("non-capturing (?:) does not create a group")
  {
    const auto pre = ustr(u"/(?:ab)c/");
    const auto str = ustr(u"abc");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE(re.parse(&str, &match));
    REQUIRE(match.cMatch == 1);
    REQUIRE(match.s[0] == 0);
    REQUIRE(match.e[0] == 3);
  }

  SECTION("numeric backreference")
  {
    require_match(u"/(a)\\1/", u"aa", 0, 2);
    require_no_match(u"/(a)\\1/", u"ab");
  }
}

TEST_CASE("CRegExp named groups", "[cregexp]")
{
  SECTION("named capture and lookup")
  {
    const auto pre = ustr(u"/(?{n}ab)c/");
    const auto str = ustr(u"abc");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    const auto name = ustr(u"n");
    REQUIRE(re.getBracketNo(&name) == 0);
    REQUIRE(*re.getBracketName(0) == name);
    REQUIRE(re.getBracketName(1) == nullptr);

    SMatches match;
    REQUIRE(re.parse(&str, &match));
    REQUIRE(match.cnMatch == 1);
    REQUIRE(match.ns[0] == 0);
    REQUIRE(match.ne[0] == 2);
  }

  SECTION("named backreference \\p{name}")
  {
    require_match(u"/(?{n}a)\\p{n}/", u"aa", 0, 2);
    require_no_match(u"/(?{n}a)\\p{n}/", u"ab");
  }

  SECTION("empty name (?{}) is non-capturing")
  {
    const auto pre = ustr(u"/(?{}ab)c/");
    const auto str = ustr(u"abc");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE(re.parse(&str, &match));
    REQUIRE(match.cnMatch == 0);
  }

  SECTION("bracket names are matched case-insensitively")
  {
    const auto pre = ustr(u"/(?{Foo}x)/");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    const auto name = ustr(u"foo");
    REQUIRE(re.getBracketNo(&name) == 0);
  }

  SECTION("unknown name")
  {
    const auto pre = ustr(u"/(?{n}x)/");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    const auto name = ustr(u"missing");
    REQUIRE(re.getBracketNo(&name) == -1);
  }
}

TEST_CASE("CRegExp lookahead and lookbehind", "[cregexp]")
{
  SECTION("positive lookahead pattern?=")
  {
    require_match(u"/abc(def)?=/", u"abcdef", 0, 3);
    require_no_match(u"/abc(def)?=/", u"abcxyz");
  }

  SECTION("negative lookahead pattern?!")
  {
    require_match(u"/abc(x)?!/", u"abc", 0, 3);
    require_no_match(u"/abc(x)?!/", u"abcx");
  }

  SECTION("positive lookbehind pattern?#N")
  {
    require_match(u"/(abc)?#3xyz/", u"abcxyz", 3, 6, false, 3);
    require_no_match(u"/(abc)?#3xyz/", u"abcxyz");
  }

  SECTION("negative lookbehind pattern?~N")
  {
    require_match(u"/(z)?~1abc/", u"xabc", 1, 4, false, 1);
    require_no_match(u"/(x)?~1abc/", u"xabc", false, 1);
  }
}

TEST_CASE("CRegExp word boundaries", "[cregexp]")
{
  require_match(u"/\\babc\\b/", u"abc", 0, 3);
  require_no_match(u"/\\babc\\b/", u"abcd");
  require_no_match(u"/\\babc\\b/", u"xabc");
  require_match(u"/\\babc\\b/", u" abc ", 1, 4, true);
  require_match(u"/\\Babc/", u"xabc", 1, 4, true);
}

TEST_CASE("CRegExp unicode character classes", "[cregexp]")
{
  require_match(u"/[{Nd}]+/", u"123", 0, 3);
  require_match(u"/[{L}]+/", u"Hello", 0, 5);
  require_no_match(u"/[{Nd}]+/", u"abc");
}

TEST_CASE("CRegExp Colorer \\m \\M and scheme start", "[cregexp]")
{
  SECTION("\\m and \\M change group 0 bounds")
  {
    const auto pre = ustr(u"/x\\mabc\\M/");
    const auto str = ustr(u"xabc");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE(re.parse(&str, &match));
    REQUIRE(match.s[0] == 1);
    REQUIRE(match.e[0] == 4);
  }

  SECTION("~ matches scheme start")
  {
    const auto pre = ustr(u"/~abc/");
    const auto str = ustr(u"xxabc");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE(re.parse(&str, 2, str.length(), &match, 2));
    REQUIRE(match.s[0] == 2);
    REQUIRE(match.e[0] == 5);
    REQUIRE_FALSE(re.parse(&str, 0, str.length(), &match, 2));
  }

  SECTION("\\m flags reset between parse() calls on the same object")
  {
    const auto pre = ustr(u"/foo\\mbar|xyz/");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;

    const auto first = ustr(u"foobar");
    REQUIRE(re.parse(&first, &match));
    REQUIRE(match.s[0] == 3);
    REQUIRE(match.e[0] == 6);

    const auto second = ustr(u"xyz");
    REQUIRE(re.parse(&second, &match));
    REQUIRE(match.s[0] == 0);
    REQUIRE(match.e[0] == 3);
  }

  SECTION("\\M flags reset between parse() calls on the same object")
  {
    const auto pre = ustr(u"/abc\\Mx|xyz/");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;

    const auto first = ustr(u"abcx");
    REQUIRE(re.parse(&first, &match));
    REQUIRE(match.s[0] == 0);
    REQUIRE(match.e[0] == 3);

    const auto second = ustr(u"xyz");
    REQUIRE(re.parse(&second, &match));
    REQUIRE(match.s[0] == 0);
    REQUIRE(match.e[0] == 3);
  }

  SECTION("reused search RE does not keep \\m from a previous line")
  {
    const auto pre = ustr(u"/foo\\mbar|end/");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;

    const auto first = ustr(u"aaafoobar");
    REQUIRE(parse_re(re, first, match, true));
    REQUIRE(match.s[0] == 6);
    REQUIRE(match.e[0] == 9);

    const auto second = ustr(u"xxxend");
    REQUIRE(parse_re(re, second, match, true));
    REQUIRE(match.s[0] == 3);
    REQUIRE(match.e[0] == 6);
  }

  SECTION("\\m does not leak between offsets inside one search")
  {
    require_match(u"/a\\mz|c/", u"abc", 2, 3, true);
  }

  SECTION("\\M does not leak between offsets inside one search")
  {
    require_match(u"/a\\Mz|c/", u"abc", 2, 3, true);
  }
}

TEST_CASE("CRegExp Colorer backtrace \\y", "[cregexp]")
{
  SECTION("numeric \\yN copies another regexp group")
  {
    const auto start_re = ustr(u"/(foo)/");
    const auto text = ustr(u"foobar");
    CRegExp start(&start_re);
    REQUIRE(start.isOk());
    SMatches start_match;
    REQUIRE(start.parse(&text, &start_match));

    const auto end_re = ustr(u"/\\y1bar/");
    CRegExp end;
    end.setBackTrace(&text, &start_match);
    REQUIRE(end.setRE(&end_re));
    SMatches end_match;
    REQUIRE(end.parse(&text, &end_match));
    REQUIRE(end_match.s[0] == 0);
    REQUIRE(end_match.e[0] == 6);
  }

  SECTION("named \\y{name} copies a named group")
  {
    const auto start_re = ustr(u"/(?{n}foo)/");
    const auto start_text = ustr(u"foo");
    CRegExp start(&start_re);
    REQUIRE(start.isOk());
    SMatches start_match;
    REQUIRE(start.parse(&start_text, &start_match));

    const auto end_re = ustr(u"/\\y{n}bar/");
    const auto end_text = ustr(u"foobar");
    CRegExp end;
    REQUIRE(end.setBackRE(&start));
    end.setBackTrace(&start_text, &start_match);
    REQUIRE(end.setRE(&end_re));
    SMatches end_match;
    REQUIRE(end.parse(&end_text, &end_match));
    REQUIRE(end_match.s[0] == 0);
    REQUIRE(end_match.e[0] == 6);
  }

  SECTION("named backtrace without setBackRE fails to compile")
  {
    const auto end_re = ustr(u"/\\y{n}/");
    CRegExp end;
    REQUIRE_FALSE(end.setRE(&end_re));
    REQUIRE(end.getError() == EError::EERROR);
  }

  SECTION("numeric \\yN outside captured groups does not read uninitialized slots")
  {
    const auto start_re = ustr(u"/(foo)/");
    const auto text = ustr(u"foo");
    CRegExp start(&start_re);
    REQUIRE(start.isOk());
    SMatches start_match;
    REQUIRE(start.parse(&text, &start_match));

    const auto end_re = ustr(u"/\\yF/");
    CRegExp end;
    end.setBackTrace(&text, &start_match);
    REQUIRE(end.setRE(&end_re));
    SMatches end_match;
    REQUIRE(end.parse(&text, &end_match));
    REQUIRE(end_match.s[0] == 0);
    REQUIRE(end_match.e[0] == 0);
  }

  SECTION("hasBackTrace is set only for \\y / \\Y operators")
  {
    const auto plain = ustr(u"/foo/");
    CRegExp plain_re(&plain);
    REQUIRE(plain_re.isOk());
    REQUIRE_FALSE(plain_re.hasBackTrace());

    const auto ynum = ustr(u"/\\y1/");
    CRegExp ynum_re(&ynum);
    REQUIRE(ynum_re.isOk());
    REQUIRE(ynum_re.hasBackTrace());

    const auto start_re = ustr(u"/(?{n}foo)/");
    CRegExp start(&start_re);
    REQUIRE(start.isOk());
    const auto yname = ustr(u"/\\Y{n}/");
    CRegExp yname_re;
    REQUIRE(yname_re.setBackRE(&start));
    REQUIRE(yname_re.setRE(&yname));
    REQUIRE(yname_re.hasBackTrace());
  }
}

TEST_CASE("CRegExp canStartWith", "[cregexp]")
{
  SECTION("literal")
  {
    const auto pre = ustr(u"/abc/");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    REQUIRE(re.canStartWith('a'));
    REQUIRE_FALSE(re.canStartWith('b'));
    REQUIRE_FALSE(re.canStartWith('x'));
  }

  SECTION("ignore case")
  {
    const auto pre = ustr(u"/abc/i");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    REQUIRE(re.canStartWith('a'));
    REQUIRE(re.canStartWith('A'));
  }

  SECTION("alternation and class")
  {
    const auto alt = ustr(u"/a|x/");
    CRegExp re_alt(&alt);
    REQUIRE(re_alt.isOk());
    REQUIRE(re_alt.canStartWith('a'));
    REQUIRE(re_alt.canStartWith('x'));
    REQUIRE_FALSE(re_alt.canStartWith('b'));

    const auto cls = ustr(u"/[xyz]/");
    CRegExp re_cls(&cls);
    REQUIRE(re_cls.isOk());
    REQUIRE(re_cls.canStartWith('y'));
    REQUIRE_FALSE(re_cls.canStartWith('a'));
  }

  SECTION("failed recompile does not leave a dangling first-char probe")
  {
    const auto good = ustr(u"/abc/");
    const auto bad = ustr(u"/(abc/");
    CRegExp re(&good);
    REQUIRE(re.isOk());
    REQUIRE(re.canStartWith('a'));
    REQUIRE_FALSE(re.setRE(&bad));
    REQUIRE_FALSE(re.isOk());
    REQUIRE(re.canStartWith('a'));
    REQUIRE(re.canStartWith('x'));
  }
}

TEST_CASE("CRegExp start anchor rejects other positions", "[cregexp]")
{
  SECTION("^ matches only at position 0")
  {
    require_match(u"/^abc/", u"abcabc", 0, 3);
    require_no_match(u"/^abc/", u"abcabc", false, 3);
    // moving search from 0 on a non-matching head: only offset 0 is tried
    require_no_match(u"/^abc/", u"xabc", true, 0);
    require_match(u"/^abc/", u"abcx", 0, 3, true, 0);
  }

  SECTION("^ inside leading brackets is still an anchor")
  {
    require_match(u"/(^\\s*)(#)/", u"  #x", 0, 3);
    require_no_match(u"/(^\\s*)(#)/", u"x #", true, 1);
  }

  SECTION("^ with /m still matches after a line break")
  {
    require_match(u"/^abc/m", u"x\nabc", 2, 5, false, 2);
    require_match(u"/^abc/m", u"x\nabc", 2, 5, true, 1);
  }

  SECTION("alternation at the front is not anchored")
  {
    require_match(u"/^abc|xyz/", u"..xyz", 2, 5, false, 2);
  }

  SECTION("~ matches only at scheme start")
  {
    const auto pre = ustr(u"/~\\s*#/");
    const auto str = ustr(u"x  #  #");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE(parse_re(re, str, match, false, 1, 1));
    REQUIRE(match.e[0] == 4);
    REQUIRE_FALSE(parse_re(re, str, match, false, 4, 1));
    REQUIRE_FALSE(parse_re(re, str, match, true, 2, 1));
  }
}

TEST_CASE("CRegExp end anchor rejects positions that cannot reach $", "[cregexp]")
{
  SECTION("$ in other positions is rejected")
  {
    require_match(u"/abc$/", u"xxabc", 2, 5, false, 2);
    require_no_match(u"/abc$/", u"xxabc", false, 0);
    require_match(u"/[^\\\\\"]$/", u"hello", 4, 5, false, 4);
    require_no_match(u"/[^\\\\\"]$/", u"hello", false, 0);
    require_match(u"/(abc)$/", u"xabc", 1, 4, false, 1);
    require_no_match(u"/(abc)$/", u"xabc", false, 0);
  }

  SECTION("/m is not end-anchored")
  {
    // With /m, $ also matches after a line break, not only at eol.
    require_match(u"/$/m", u"a\nb", 2, 2, true, 0);
    require_match(u"/$/m", u"a\nb", 2, 2, false, 2);
  }

  SECTION(".*$ is unbounded and is not filtered")
  {
    require_match(u"/.*$/", u"abcd", 0, 4, true, 0);
    require_match(u"/a.*$/", u"a___", 0, 4, true, 0);
  }

  SECTION("positionMoves starts at end - maxLen")
  {
    require_match(u"/abc$/", u"xxabc", 2, 5, true, 0);
    require_match(u"/x$/", u"abcx", 3, 4, true, 0);
    require_match(u"/[^\\\\\"]$/", u"hello", 4, 5, true, 0);
  }

  SECTION("\\y1$ is unbounded and is not filtered")
  {
    const auto start_re = ustr(u"/(abc)/");
    const auto text = ustr(u"xxabc");
    CRegExp start(&start_re);
    REQUIRE(start.isOk());
    SMatches start_match;
    REQUIRE(start.parse(&text, 2, text.length(), &start_match));

    const auto end_re = ustr(u"/\\y1$/");
    CRegExp end;
    end.setBackTrace(&text, &start_match);
    REQUIRE(end.setRE(&end_re));
    SMatches end_match;
    REQUIRE(end.parse(&text, 0, text.length(), &end_match, 0, 1));
    REQUIRE(end_match.s[0] == 2);
    REQUIRE(end_match.e[0] == 5);
  }

  SECTION("eol is the $ bound, not the string length")
  {
    const auto pre = ustr(u"/a$/");
    const auto str = ustr(u"xaZ");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    AsciiCharMask chars;
    CRegExp::collectAsciiChars(str, chars);
    REQUIRE_FALSE(re.mayMatch(0, 2, 0, chars));
    REQUIRE(re.mayMatch(1, 2, 0, chars));
    REQUIRE_FALSE(re.parse(&str, 0, 2, &match));
    REQUIRE(re.parse(&str, 1, 2, &match));
    REQUIRE(match.s[0] == 1);
    REQUIRE(match.e[0] == 2);
    REQUIRE_FALSE(re.parse(&str, 1, 3, &match));
  }

  SECTION("(^|[^\\\\]?#1)$ matches only at eol")
  {
    const auto pre = ustr(u"/(^|[^\\\\]?#1)$/");
    const auto str = ustr(u"abc");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    AsciiCharMask chars;
    CRegExp::collectAsciiChars(str, chars);
    REQUIRE(re.mayMatch(3, 3, 0, chars));
    REQUIRE_FALSE(re.mayMatch(0, 3, 0, chars));
    REQUIRE(parse_re(re, str, match, false, 3));
    REQUIRE(match.s[0] == 3);
    REQUIRE(match.e[0] == 3);
    REQUIRE_FALSE(parse_re(re, str, match, false, 0));
    REQUIRE(parse_re(re, str, match, true, 0));
    REQUIRE(match.s[0] == 3);
    REQUIRE(match.e[0] == 3);

    const auto escaped = ustr(u"ab\\");
    REQUIRE_FALSE(parse_re(re, escaped, match, false, 3));
    require_match(u"/(^|[^\\\\]?#1)$/", u"", 0, 0);
  }

  SECTION("top-level alternation is not end-anchored")
  {
    require_match(u"/abc$|xyz/", u"..xyz", 2, 5, false, 2);
  }
}

TEST_CASE("CRegExp required characters prefilter", "[cregexp]")
{
  auto parse_with_line = [](const char16_t* pattern, const char16_t* text, int pos = 0) {
    const auto pre = ustr(pattern);
    const auto str = ustr(text);
    CRegExp re(&pre);
    INFO("pattern: " << UStr::to_stdstr(&pre) << " text: " << UStr::to_stdstr(&str));
    REQUIRE(re.isOk());
    AsciiCharMask chars;
    CRegExp::collectAsciiChars(str, chars);
    SMatches match;
    return re.parse(&str, pos, str.length(), &match, 0, 1, &chars);
  };

  SECTION("collectAsciiChars ignores non-ASCII")
  {
    AsciiCharMask chars;
    CRegExp::collectAsciiChars(ustr(u"a@\u00e9"), chars);
    REQUIRE((chars[1] & (uint64_t(1) << ('a' - 64))) != 0);
    REQUIRE((chars[1] & (uint64_t(1) << ('@' - 64))) != 0);
    REQUIRE((chars[1] & (uint64_t(1) << ('b' - 64))) == 0);
    REQUIRE(chars[0] == 0);
  }

  SECTION("literal present and absent")
  {
    REQUIRE(parse_with_line(u"/\\w+\\@\\w+/", u"mail me@host"));
    REQUIRE_FALSE(parse_with_line(u"/\\w+\\@\\w+/", u"mail me at host"));
    REQUIRE(parse_with_line(u"/x/", u"x"));
  }

  SECTION("optional and repeated-zero groups are not required")
  {
    REQUIRE(parse_with_line(u"/(x\\@)?y/", u"y"));
    REQUIRE(parse_with_line(u"/(\\@)*y/", u"y"));
    REQUIRE(parse_with_line(u"/(\\@){0,3}y/", u"y"));
    REQUIRE(parse_with_line(u"/\\@?y/", u"y"));
  }

  SECTION("alternation requires one branch only")
  {
    REQUIRE(parse_with_line(u"/(a\\@|b\\#)/", u"b#"));
    REQUIRE(parse_with_line(u"/a\\@|b\\#/", u"a@"));
    REQUIRE(parse_with_line(u"/(x|)\\%/", u"%"));
    REQUIRE_FALSE(parse_with_line(u"/(a\\@|b\\#)/", u"ab"));
  }

  SECTION("classes, meta symbols and lookarounds do not add requirements")
  {
    REQUIRE(parse_with_line(u"/[\\@]?y/", u"y"));
    REQUIRE(parse_with_line(u"/\\d?y/", u"y"));
    REQUIRE(parse_with_line(u"/y(\\@)?!/", u"y"));
    REQUIRE(parse_with_line(u"/(\\@)?~1y/", u"y"));
    REQUIRE(parse_with_line(u"/(y)\\1/", u"yy"));
  }

  SECTION("positive lookahead content is required")
  {
    REQUIRE(parse_with_line(u"/y(\\@)?=/", u"y@"));
    REQUIRE_FALSE(parse_with_line(u"/y(\\@)?=/", u"y"));
  }

  SECTION("case-insensitive letters are not required")
  {
    REQUIRE(parse_with_line(u"/k/i", u"K"));
    REQUIRE(parse_with_line(u"/a1/i", u"A1"));
    REQUIRE_FALSE(parse_with_line(u"/a1/i", u"A2"));
  }

  SECTION("line mask covers text outside [pos, eol)")
  {
    const auto pre = ustr(u"/\\@/");
    const auto str = ustr(u"@ x");
    CRegExp re(&pre);
    AsciiCharMask chars;
    CRegExp::collectAsciiChars(str, chars);
    SMatches match;
    REQUIRE_FALSE(re.parse(&str, 1, str.length(), &match, 0, 0, &chars));
    REQUIRE(re.parse(&str, 0, str.length(), &match, 0, 0, &chars));
  }

  SECTION("no mask keeps the plain matcher")
  {
    require_no_match(u"/\\w+\\@\\w+/", u"mail me at host");
    require_match(u"/\\w+\\@\\w+/", u"me@host", 0, 7);
  }
}

TEST_CASE("CRegExp alternation first-char dispatcher", "[cregexp]")
{
  SECTION("left alternative wins even when a longer right one also matches")
  {
    require_match(u"/(a|ab)/", u"ab", 0, 1);
  }

  SECTION("empty alternative")
  {
    require_match(u"/(a|)/", u"a", 0, 1);
    require_match(u"/(a|)/", u"b", 0, 0);
    require_match(u"/(a|)/", u"", 0, 0);
    require_match(u"/(|a)/", u"a", 0, 0);
  }

  SECTION("ignore case")
  {
    require_match(u"/(abc|xyz)/i", u"ABC", 0, 3);
    require_match(u"/(abc|xyz)/i", u"XyZ", 0, 3);
    require_no_match(u"/(abc|xyz)/i", u"qrs");
  }

  SECTION("non-ASCII text takes the unfiltered path")
  {
    require_no_match(u"/(a|b)/", u"\u00e9");
    require_match(u"/(a|b)/", u"\u00e9a", 1, 2, true);
    require_match(u"/(\u00e9|b)/", u"\u00e9", 0, 1);
    require_match(u"/(\u00e9|b)/", u"b", 0, 1);
  }

  SECTION("zero-width prefix is nullable and is not dispatched")
  {
    require_match(u"/(\\b|x)/", u"a", 0, 0);
    require_match(u"/(\\bfoo|bar)/", u"foo", 0, 3);
    require_match(u"/(\\bfoo|bar)/", u"bar", 0, 3);
    require_match(u"/(x?=|a)/", u"x", 0, 0);
    require_match(u"/(x?=|a)/", u"a", 0, 1);
  }

  SECTION("\\M in a failed alternative still bounds group 0")
  {
    const auto pre = ustr(u"/\\M\\s+|;/");
    const auto str = ustr(u";");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE(re.parse(&str, &match));
    REQUIRE(match.s[0] == 0);
    REQUIRE(match.e[0] == 0);
  }

  SECTION("nested alternation")
  {
    require_match(u"/((a|b)|c)/", u"a", 0, 1);
    require_match(u"/((a|b)|c)/", u"b", 0, 1);
    require_match(u"/((a|b)|c)/", u"c", 0, 1);
    require_no_match(u"/((a|b)|c)/", u"d");
  }
}

TEST_CASE("CRegExp stack reuse after clearRegExpStack", "[cregexp]")
{
  const auto pre = ustr(u"/(a|b)+c/");
  const auto str = ustr(u"aaabbc");
  CRegExp re(&pre);
  REQUIRE(re.isOk());
  SMatches match;
  REQUIRE(re.parse(&str, &match));
  CRegExp::clearRegExpStack();
  REQUIRE(re.parse(&str, &match));
  REQUIRE(match.s[0] == 0);
  REQUIRE(match.e[0] == 6);
}

TEST_CASE("ParserFactory on the same thread does not clear the matcher stack", "[cregexp]")
{
  const auto pre = ustr(u"/(a|b)+c/");
  const auto str = ustr(u"aaabbc");
  CRegExp re(&pre);
  REQUIRE(re.isOk());
  SMatches match;
  REQUIRE(re.parse(&str, &match));
  {
    ParserFactory probe;
  }
  REQUIRE(re.parse(&str, &match));
  REQUIRE(match.s[0] == 0);
  REQUIRE(match.e[0] == 6);
}

TEST_CASE("CRegExp \\Y{name} copies named group case-insensitively", "[cregexp]")
{
  const auto start_re = ustr(u"/(x)(?{n}Foo)/");
  const auto start_text = ustr(u"xFoo");
  CRegExp start(&start_re);
  REQUIRE(start.isOk());
  SMatches start_match;
  REQUIRE(start.parse(&start_text, &start_match));

  const auto end_re = ustr(u"/\\Y{n}/");
  const auto end_text = ustr(u"foo");
  CRegExp end;
  REQUIRE(end.setBackRE(&start));
  end.setBackTrace(&start_text, &start_match);
  REQUIRE(end.setRE(&end_re));
  SMatches end_match;
  REQUIRE(end.parse(&end_text, &end_match));
  REQUIRE(end_match.s[0] == 0);
  REQUIRE(end_match.e[0] == 3);
}

TEST_CASE("CRegExp parse-step budget stops catastrophic backtracking", "[cregexp]")
{
  SECTION("nested plus against a non-matching tail fails the budget")
  {
    const auto pre = ustr(u"/(a+)+b/");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    re.setParseStepLimit(10000);
    UnicodeString text;
    for (int i = 0; i < 24; i++) {
      text.append('a');
    }
    text.append('c');
    SMatches match;
    REQUIRE_FALSE(parse_re(re, text, match));
    REQUIRE(re.exceededParseStepLimit());
  }

  SECTION("a normal match does not trip the budget")
  {
    const auto pre = ustr(u"/abc/");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE(parse_re(re, ustr(u"abc"), match));
    REQUIRE_FALSE(re.exceededParseStepLimit());
    REQUIRE(match.s[0] == 0);
    REQUIRE(match.e[0] == 3);
  }

  SECTION("ordinary miss does not trip the budget")
  {
    const auto pre = ustr(u"/xyz/");
    CRegExp re(&pre);
    REQUIRE(re.isOk());
    SMatches match;
    REQUIRE_FALSE(parse_re(re, ustr(u"aaa"), match));
    REQUIRE_FALSE(re.exceededParseStepLimit());
  }
}
