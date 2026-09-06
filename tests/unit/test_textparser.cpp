#include <catch2/catch_amalgamated.hpp>
#include <vector>
#include "colorer/HrcLibrary.h"
#include "colorer/LineSource.h"
#include "colorer/Region.h"
#include "colorer/RegionHandler.h"
#include "colorer/Scheme.h"
#include "colorer/TextParser.h"
#include "colorer/utils/FileSystems.h"
#include "colorer/xml/XmlInputSource.h"

namespace {

struct RegionHit
{
  int start = 0;
  int end = 0;
  UnicodeString name;
};

struct SchemeEvent
{
  bool enter = false;
  size_t line = 0;
  int start = 0;
  int end = 0;
  UnicodeString scheme;
};

class CollectHandler : public RegionHandler
{
 public:
  std::vector<RegionHit> hits;
  std::vector<SchemeEvent> schemes;

  void addRegion(size_t /*lno*/, UnicodeString* /*line*/, int sx, int ex, const Region* region) override
  {
    hits.push_back({sx, ex, region->getName()});
  }

  void enterScheme(size_t lno, UnicodeString* /*line*/, int sx, int ex, const Region* /*region*/,
                   const Scheme* scheme) override
  {
    schemes.push_back({true, lno, sx, ex, *scheme->getName()});
  }

  void leaveScheme(size_t lno, UnicodeString* /*line*/, int sx, int ex, const Region* /*region*/,
                   const Scheme* scheme) override
  {
    schemes.push_back({false, lno, sx, ex, *scheme->getName()});
  }
};

class SingleLineSource : public LineSource
{
 public:
  explicit SingleLineSource(UnicodeString line) : line_(std::move(line)) {}

  UnicodeString* getLine(size_t lno) override
  {
    return lno == 0 ? &line_ : nullptr;
  }

 private:
  UnicodeString line_;
};

class InvalidatingLineSource : public LineSource
{
 public:
  explicit InvalidatingLineSource(std::vector<UnicodeString> lines) : lines_(std::move(lines)) {}

  UnicodeString* getLine(size_t lno) override
  {
    if (lno >= lines_.size()) {
      return nullptr;
    }
    current_ = lines_[lno];
    return &current_;
  }

 private:
  std::vector<UnicodeString> lines_;
  UnicodeString current_;
};

std::vector<RegionHit> parseLine(HrcLibrary& lib, const UnicodeString& type_name, const UnicodeString& line,
                                 bool chunk_long_lines = false)
{
  auto* file_type = lib.getFileType(type_name);
  REQUIRE(file_type != nullptr);
  REQUIRE(file_type->getBaseScheme() != nullptr);

  SingleLineSource source(line);
  CollectHandler handler;
  TextParser parser;
  parser.setFileType(file_type);
  parser.setLineSource(&source);
  parser.setRegionHandler(&handler);
  parser.setChunkLongLines(chunk_long_lines);
  parser.parse(0, 1, TextParser::TextParseMode::TPM_CACHE_OFF);
  return handler.hits;
}

struct ParseResult
{
  std::vector<RegionHit> hits;
  std::vector<SchemeEvent> schemes;
};

ParseResult parseLines(HrcLibrary& lib, const UnicodeString& type_name, std::vector<UnicodeString> lines,
                       TextParser::TextParseMode mode)
{
  auto* file_type = lib.getFileType(type_name);
  REQUIRE(file_type != nullptr);
  REQUIRE(file_type->getBaseScheme() != nullptr);

  const auto num = static_cast<int>(lines.size());
  InvalidatingLineSource source(std::move(lines));
  CollectHandler handler;
  TextParser parser;
  parser.setFileType(file_type);
  parser.setLineSource(&source);
  parser.setRegionHandler(&handler);
  parser.parse(0, num, mode);
  return {std::move(handler.hits), std::move(handler.schemes)};
}

}  // namespace

TEST_CASE("Keyword matching still works with lazy line lowercase", "[textparser]")
{
  auto hrc_path = fs::path(__FILE__).parent_path() / "data" / "type_kwcase.hrc";
  XmlInputSource input(UnicodeString(hrc_path.c_str()));
  HrcLibrary lib;
  lib.loadSource(&input);

  SECTION("case-sensitive keywords match only the stored case")
  {
    const auto hits = parseLine(lib, UnicodeString("kw_sensitive"), UnicodeString(u"If if"));
    REQUIRE(hits.size() == 1);
    REQUIRE(hits[0].start == 0);
    REQUIRE(hits[0].end == 2);
    REQUIRE(hits[0].name.compare(UnicodeString("kw_sensitive:Keyword")) == 0);
  }

  SECTION("ignorecase keywords match both cases")
  {
    const auto hits = parseLine(lib, UnicodeString("kw_ignore"), UnicodeString(u"If if"));
    REQUIRE(hits.size() == 2);
    REQUIRE(hits[0].start == 0);
    REQUIRE(hits[0].end == 2);
    REQUIRE(hits[1].start == 3);
    REQUIRE(hits[1].end == 5);
    REQUIRE(hits[0].name.compare(UnicodeString("kw_ignore:Keyword")) == 0);
    REQUIRE(hits[1].name.compare(UnicodeString("kw_ignore:Keyword")) == 0);
  }

  SECTION("lowercase is built even if ignorecase keywords are not first")
  {
    const auto hits = parseLine(lib, UnicodeString("kw_mixed"), UnicodeString(u"If Then if then"));
    REQUIRE(hits.size() == 3);
    REQUIRE(hits[0].start == 0);
    REQUIRE(hits[0].end == 2);
    REQUIRE(hits[0].name.compare(UnicodeString("kw_mixed:Kw")) == 0);
    REQUIRE(hits[1].start == 3);
    REQUIRE(hits[1].end == 7);
    REQUIRE(hits[1].name.compare(UnicodeString("kw_mixed:KwI")) == 0);
    REQUIRE(hits[2].start == 11);
    REQUIRE(hits[2].end == 15);
    REQUIRE(hits[2].name.compare(UnicodeString("kw_mixed:KwI")) == 0);
  }
}

TEST_CASE("Block end copies the start line only when it uses backtrace", "[textparser]")
{
  auto hrc_path = fs::path(__FILE__).parent_path() / "data" / "type_block.hrc";
  XmlInputSource input(UnicodeString(hrc_path.c_str()));
  HrcLibrary lib;
  lib.loadSource(&input);

  SECTION("quoted string without \\y still finds the end")
  {
    const auto hits = parseLine(lib, UnicodeString("bl_quote"), UnicodeString(u"'ab'"));
    REQUIRE(hits.size() == 1);
    REQUIRE(hits[0].start == 1);
    REQUIRE(hits[0].end == 3);
    REQUIRE(hits[0].name.compare(UnicodeString("bl_quote:Content")) == 0);
  }

  SECTION("named \\y closes with the same quote as start")
  {
    const auto hits = parseLine(lib, UnicodeString("bl_yquote"), UnicodeString(u"'ab'\"cd\""));
    REQUIRE(hits.size() == 2);
    REQUIRE(hits[0].start == 1);
    REQUIRE(hits[0].end == 3);
    REQUIRE(hits[0].name.compare(UnicodeString("bl_yquote:Content")) == 0);
    REQUIRE(hits[1].start == 5);
    REQUIRE(hits[1].end == 7);
    REQUIRE(hits[1].name.compare(UnicodeString("bl_yquote:Content")) == 0);
  }

  SECTION("multiline \\y still sees the start line after getLine invalidates it")
  {
    const auto result = parseLines(lib, UnicodeString("bl_here"),
                                    {UnicodeString(u"<<END"), UnicodeString(u"foo"), UnicodeString(u"END")},
                                    TextParser::TextParseMode::TPM_CACHE_OFF);
    REQUIRE(result.hits.size() == 1);
    REQUIRE(result.hits[0].start == 0);
    REQUIRE(result.hits[0].end == 3);
    REQUIRE(result.hits[0].name.compare(UnicodeString("bl_here:Content")) == 0);
    REQUIRE_FALSE(result.schemes.empty());
    REQUIRE(result.schemes.front().enter);
    bool left_on_end = false;
    for (const auto& event : result.schemes) {
      if (!event.enter && event.line == 2 && event.start == 0 && event.end == 3) {
        left_on_end = true;
      }
    }
    REQUIRE(left_on_end);
  }

  SECTION("cached multiline \\y keeps the start-line copy")
  {
    const auto result = parseLines(lib, UnicodeString("bl_here"),
                                    {UnicodeString(u"<<END"), UnicodeString(u"foo"), UnicodeString(u"END")},
                                    TextParser::TextParseMode::TPM_CACHE_UPDATE);
    REQUIRE(result.hits.size() == 1);
    bool left_on_end = false;
    for (const auto& event : result.schemes) {
      if (!event.enter && event.line == 2 && event.end == 3) {
        left_on_end = true;
      }
    }
    REQUIRE(left_on_end);
  }
}

TEST_CASE("Inherit virtual substitution still maps schemes", "[textparser]")
{
  auto hrc_path = fs::path(__FILE__).parent_path() / "data" / "type_virtual.hrc";
  XmlInputSource input(UnicodeString(hrc_path.c_str()));
  HrcLibrary lib;
  lib.loadSource(&input);

  SECTION("empty virtual stack uses the inherited scheme as-is")
  {
    const auto hits = parseLine(lib, UnicodeString("virt_plain"), UnicodeString(u"foo bar"));
    REQUIRE(hits.size() == 1);
    REQUIRE(hits[0].start == 0);
    REQUIRE(hits[0].end == 3);
    REQUIRE(hits[0].name.compare(UnicodeString("virt_plain:BaseKw")) == 0);
  }

  SECTION("virtual replaces the target scheme")
  {
    const auto hits = parseLine(lib, UnicodeString("virt_subst"), UnicodeString(u"foo bar"));
    REQUIRE(hits.size() == 1);
    REQUIRE(hits[0].start == 4);
    REQUIRE(hits[0].end == 7);
    REQUIRE(hits[0].name.compare(UnicodeString("virt_subst:AltKw")) == 0);
  }

  SECTION("virtual replaces a block scheme")
  {
    const auto hits = parseLine(lib, UnicodeString("virt_block"), UnicodeString(u"[bar]"));
    REQUIRE(hits.size() == 1);
    REQUIRE(hits[0].start == 1);
    REQUIRE(hits[0].end == 4);
    REQUIRE(hits[0].name.compare(UnicodeString("virt_block:AltKw")) == 0);
  }

  SECTION("cached multiline block keeps the virtual substitution")
  {
    auto* file_type = lib.getFileType(UnicodeString("virt_block"));
    REQUIRE(file_type != nullptr);
    REQUIRE(file_type->getBaseScheme() != nullptr);

    InvalidatingLineSource source({UnicodeString(u"["), UnicodeString(u"bar"), UnicodeString(u"]")});
    CollectHandler handler;
    TextParser parser;
    parser.setFileType(file_type);
    parser.setLineSource(&source);
    parser.setRegionHandler(&handler);
    parser.parse(0, 1, TextParser::TextParseMode::TPM_CACHE_UPDATE);
    parser.parse(1, 2, TextParser::TextParseMode::TPM_CACHE_READ);

    REQUIRE(handler.hits.size() == 1);
    REQUIRE(handler.hits[0].start == 0);
    REQUIRE(handler.hits[0].end == 3);
    REQUIRE(handler.hits[0].name.compare(UnicodeString("virt_block:AltKw")) == 0);
  }
}

TEST_CASE("Inherited keyword lists flatten and merge without changing matches", "[textparser]")
{
  auto hrc_path = fs::path(__FILE__).parent_path() / "data" / "type_kwmerge.hrc";
  XmlInputSource input(UnicodeString(hrc_path.c_str()));
  HrcLibrary lib;
  lib.loadSource(&input);

  SECTION("two-level inherit still finds the inner keyword")
  {
    const auto hits = parseLine(lib, UnicodeString("kw_nested"), UnicodeString(u"foo x"));
    REQUIRE(hits.size() == 1);
    REQUIRE(hits[0].start == 0);
    REQUIRE(hits[0].end == 3);
    REQUIRE(hits[0].name.compare(UnicodeString("kw_nested:Inner")) == 0);
  }

  SECTION("the first inherited list wins a duplicate word")
  {
    const auto hits = parseLine(lib, UnicodeString("kw_join"), UnicodeString(u"echo"));
    REQUIRE(hits.size() == 1);
    REQUIRE(hits[0].start == 0);
    REQUIRE(hits[0].end == 4);
    REQUIRE(hits[0].name.compare(UnicodeString("kw_join:First")) == 0);
  }

  SECTION("a longer word from a later list still matches")
  {
    const auto hits = parseLine(lib, UnicodeString("kw_join"), UnicodeString(u"echolong aa bb"));
    REQUIRE(hits.size() == 3);
    REQUIRE(hits[0].name.compare(UnicodeString("kw_join:Second")) == 0);
    REQUIRE(hits[0].start == 0);
    REQUIRE(hits[0].end == 8);
    REQUIRE(hits[1].name.compare(UnicodeString("kw_join:First")) == 0);
    REQUIRE(hits[1].start == 9);
    REQUIRE(hits[1].end == 11);
    REQUIRE(hits[2].name.compare(UnicodeString("kw_join:Second")) == 0);
    REQUIRE(hits[2].start == 12);
    REQUIRE(hits[2].end == 14);
  }

  SECTION("a regexp between keyword lists is not skipped")
  {
    const auto hits = parseLine(lib, UnicodeString("kw_split"), UnicodeString(u"hello ho hi"));
    REQUIRE(hits.size() == 3);
    REQUIRE(hits[0].name.compare(UnicodeString("kw_split:Re")) == 0);
    REQUIRE(hits[0].start == 0);
    REQUIRE(hits[0].end == 5);
    REQUIRE(hits[1].name.compare(UnicodeString("kw_split:Kw2")) == 0);
    REQUIRE(hits[1].start == 6);
    REQUIRE(hits[1].end == 8);
    REQUIRE(hits[2].name.compare(UnicodeString("kw_split:Kw")) == 0);
    REQUIRE(hits[2].start == 9);
    REQUIRE(hits[2].end == 11);
  }
}

TEST_CASE("tryParseLine accepts content edits and rejects block-boundary edits", "[textparser]")
{
  auto hrc_path = fs::path(__FILE__).parent_path() / "data" / "type_tryline.hrc";
  XmlInputSource input(UnicodeString(hrc_path.c_str()));
  HrcLibrary lib;
  lib.loadSource(&input);
  auto* file_type = lib.getFileType(UnicodeString("try_line"));
  REQUIRE(file_type != nullptr);
  REQUIRE(file_type->getBaseScheme() != nullptr);

  InvalidatingLineSource source(
      {UnicodeString(u"int a"), UnicodeString(u"/*"), UnicodeString(u" cmt"), UnicodeString(u"*/"),
       UnicodeString(u"int b")});
  CollectHandler handler;
  TextParser parser;
  parser.setFileType(file_type);
  parser.setLineSource(&source);
  parser.setRegionHandler(&handler);
  parser.parse(0, 5, TextParser::TextParseMode::TPM_CACHE_UPDATE);

  SECTION("content inside a comment keeps the scheme stack")
  {
    REQUIRE(parser.tryParseLine(2));
  }

  SECTION("closing the comment on a middle line changes the stack")
  {
    InvalidatingLineSource closed(
        {UnicodeString(u"int a"), UnicodeString(u"/*"), UnicodeString(u" cmt */"),
         UnicodeString(u"*/"), UnicodeString(u"int b")});
    parser.setLineSource(&closed);
    REQUIRE_FALSE(parser.tryParseLine(2));
  }
}

TEST_CASE("Chunked cache updates keep searchLine covering the right block", "[textparser]")
{
  auto hrc_path = fs::path(__FILE__).parent_path() / "data" / "type_tryline.hrc";
  XmlInputSource input(UnicodeString(hrc_path.c_str()));
  HrcLibrary lib;
  lib.loadSource(&input);
  auto* file_type = lib.getFileType(UnicodeString("try_line"));
  REQUIRE(file_type != nullptr);
  REQUIRE(file_type->getBaseScheme() != nullptr);

  constexpr int kBlocks = 40;
  std::vector<UnicodeString> lines;
  lines.reserve(static_cast<size_t>(kBlocks) * 4);
  for (int i = 0; i < kBlocks; i++) {
    lines.emplace_back(u"int x");
    lines.emplace_back(u"/*");
    lines.emplace_back(u" cmt");
    lines.emplace_back(u"*/");
  }
  const int n = static_cast<int>(lines.size());
  const int last_cmt = (kBlocks - 1) * 4 + 2;

  InvalidatingLineSource source(std::move(lines));
  CollectHandler handler;
  TextParser parser;
  parser.setFileType(file_type);
  parser.setLineSource(&source);
  parser.setRegionHandler(&handler);

  int pos = 0;
  while (pos < n) {
    const int chunk = n - pos < 3 ? n - pos : 3;
    pos = parser.parse(pos, chunk, TextParser::TextParseMode::TPM_CACHE_UPDATE) + 1;
  }

  SECTION("tryParseLine still matches the stack in an early and a late comment")
  {
    REQUIRE(parser.tryParseLine(2));
    REQUIRE(parser.tryParseLine(last_cmt));
    REQUIRE(parser.tryParseLine(0));
  }

  SECTION("CACHE_READ after warming to EOF still colors the first line")
  {
    handler.hits.clear();
    parser.parse(0, 1, TextParser::TextParseMode::TPM_CACHE_READ);
    REQUIRE_FALSE(handler.hits.empty());
    REQUIRE(handler.hits.front().name.compare(UnicodeString("try_line:Kw")) == 0);
  }

  SECTION("CACHE_READ of a late comment after a jump to line 0 stays in that block")
  {
    handler.hits.clear();
    parser.parse(0, 1, TextParser::TextParseMode::TPM_CACHE_READ);
    handler.hits.clear();
    parser.parse(last_cmt, 1, TextParser::TextParseMode::TPM_CACHE_READ);
    REQUIRE_FALSE(handler.hits.empty());
    REQUIRE(handler.hits.front().name.compare(UnicodeString("try_line:Cmt")) == 0);
  }
}

TEST_CASE("Long lines keep coloring past the maxBlockSize window", "[textparser]")
{
  auto hrc_path = fs::path(__FILE__).parent_path() / "data" / "type_longline.hrc";
  XmlInputSource input(UnicodeString(hrc_path.c_str()));
  HrcLibrary lib;
  lib.loadSource(&input);

  UnicodeString prefix;
  for (int i = 0; i < 1100; i++) {
    prefix.append(' ');
  }

  SECTION("by default the rest of the line after one window is skipped")
  {
    UnicodeString line = prefix;
    line.append(UnicodeString(u"If"));
    const auto hits = parseLine(lib, UnicodeString("long_line"), line);
    REQUIRE(hits.empty());
  }

  SECTION("keyword after the first 1000 characters is still matched")
  {
    UnicodeString line = prefix;
    line.append(UnicodeString(u"If"));
    const auto hits = parseLine(lib, UnicodeString("long_line"), line, true);
    REQUIRE(hits.size() == 1);
    REQUIRE(hits[0].start == 1100);
    REQUIRE(hits[0].end == 1102);
    REQUIRE(hits[0].name.compare(UnicodeString("long_line:Keyword")) == 0);
  }

  SECTION("regexp after the first 1000 characters is still matched")
  {
    UnicodeString line = prefix;
    line.append(UnicodeString(u"Zzz"));
    const auto hits = parseLine(lib, UnicodeString("long_line"), line, true);
    REQUIRE(hits.size() == 1);
    REQUIRE(hits[0].start == 1100);
    REQUIRE(hits[0].end == 1103);
    REQUIRE(hits[0].name.compare(UnicodeString("long_line:Keyword")) == 0);
  }
}
