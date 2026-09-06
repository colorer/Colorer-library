#include <catch2/catch_amalgamated.hpp>
#include <vector>
#include "colorer/HrcLibrary.h"
#include "colorer/LineSource.h"
#include "colorer/ParserFactory.h"
#include "colorer/RegionHandler.h"
#include "colorer/editor/BaseEditor.h"
#include "colorer/FileType.h"
#include "colorer/utils/FileSystems.h"
#include "colorer/xml/XmlInputSource.h"

namespace {

class MutableLines : public LineSource
{
 public:
  explicit MutableLines(std::vector<UnicodeString> lines) : lines_(std::move(lines)) {}

  UnicodeString* getLine(size_t lno) override
  {
    return lno < lines_.size() ? &lines_[lno] : nullptr;
  }

  void setLine(size_t lno, UnicodeString text)
  {
    REQUIRE(lno < lines_.size());
    lines_[lno] = std::move(text);
  }

  int size() const
  {
    return static_cast<int>(lines_.size());
  }

 private:
  std::vector<UnicodeString> lines_;
};

class CountHandler : public RegionHandler
{
 public:
  int clear_lines = 0;

  void addRegion(size_t /*lno*/, UnicodeString* /*line*/, int /*sx*/, int /*ex*/,
                 const Region* /*region*/) override
  {
  }

  void enterScheme(size_t /*lno*/, UnicodeString* /*line*/, int /*sx*/, int /*ex*/,
                   const Region* /*region*/, const Scheme* /*scheme*/) override
  {
  }

  void leaveScheme(size_t /*lno*/, UnicodeString* /*line*/, int /*sx*/, int /*ex*/,
                   const Region* /*region*/, const Scheme* /*scheme*/) override
  {
  }

  void clearLine(size_t /*lno*/, UnicodeString* /*line*/) override
  {
    clear_lines++;
  }
};

bool hasRegion(const LineRegion* regions, const char* name)
{
  const UnicodeString want(name);
  for (const LineRegion* lr = regions; lr != nullptr; lr = lr->next) {
    if (lr->region != nullptr && lr->region->getName().compare(want) == 0) {
      return true;
    }
  }
  return false;
}

HrcLibrary& loadTryLine(ParserFactory& factory)
{
  auto hrc_path = fs::path(__FILE__).parent_path() / "data" / "type_tryline.hrc";
  UnicodeString location(hrc_path.c_str());
  factory.loadHrcPath(&location);
  return factory.getHrcLibrary();
}

std::unique_ptr<BaseEditor> makeEditor(ParserFactory& factory, MutableLines& source)
{
  auto editor = std::make_unique<BaseEditor>(&factory, &source);
  auto* type = factory.getHrcLibrary().getFileType(UnicodeString("try_line"));
  REQUIRE(type != nullptr);
  editor->setFileType(type);
  editor->lineCountEvent(source.size());
  editor->visibleTextEvent(0, source.size());
  editor->validate(-1, true);
  return editor;
}

}  // namespace

TEST_CASE("modifyLineEvent keeps the tail valid when the scheme stack is unchanged", "[baseeditor]")
{
  ParserFactory factory;
  loadTryLine(factory);

  MutableLines source({UnicodeString(u"int a"), UnicodeString(u"/*"), UnicodeString(u" cmt"),
                       UnicodeString(u"*/"), UnicodeString(u"int b")});
  auto editor = makeEditor(factory, source);
  const int parsed = source.size();
  REQUIRE(editor->getInvalidLine() == parsed);

  CountHandler counter;
  editor->addRegionHandler(&counter);

  SECTION("edit inside a comment does not invalidate following lines")
  {
    source.setLine(2, UnicodeString(u" cmt2"));
    counter.clear_lines = 0;
    editor->modifyLineEvent(2);
    REQUIRE(counter.clear_lines == 1);
    REQUIRE(editor->getInvalidLine() == parsed);

    counter.clear_lines = 0;
    const LineRegion* regions = editor->getLineRegions(4);
    REQUIRE(counter.clear_lines == 0);
    REQUIRE(editor->getInvalidLine() == parsed);
    bool saw_int = false;
    for (const LineRegion* lr = regions; lr != nullptr; lr = lr->next) {
      if (lr->region != nullptr && lr->region->getName().compare(UnicodeString("try_line:Kw")) == 0) {
        saw_int = true;
      }
    }
    REQUIRE(saw_int);
  }

  SECTION("closing a comment early invalidates the tail")
  {
    source.setLine(2, UnicodeString(u" cmt */"));
    counter.clear_lines = 0;
    editor->modifyLineEvent(2);
    REQUIRE(counter.clear_lines == 1);
    REQUIRE(editor->getInvalidLine() == 2);

    editor->validate(-1, true);
    REQUIRE(editor->getInvalidLine() == parsed);
    const LineRegion* regions = editor->getLineRegions(4);
    bool saw_int = false;
    for (const LineRegion* lr = regions; lr != nullptr; lr = lr->next) {
      if (lr->region != nullptr && lr->region->getName().compare(UnicodeString("try_line:Kw")) == 0) {
        saw_int = true;
      }
    }
    REQUIRE(saw_int);
  }

  SECTION("opening a new block invalidates the tail")
  {
    source.setLine(0, UnicodeString(u"int a /*"));
    editor->modifyLineEvent(0);
    REQUIRE(editor->getInvalidLine() == 0);
  }

  SECTION("changing a heredoc tag is a structural change")
  {
    MutableLines here({UnicodeString(u"<<END"), UnicodeString(u"foo"), UnicodeString(u"END"),
                       UnicodeString(u"int x")});
    auto here_editor = makeEditor(factory, here);
    REQUIRE(here_editor->getInvalidLine() == here.size());
    here.setLine(0, UnicodeString(u"<<FOO"));
    here_editor->modifyLineEvent(0);
    REQUIRE(here_editor->getInvalidLine() == 0);
  }

  SECTION("edit inside a heredoc does not invalidate the tail")
  {
    MutableLines here({UnicodeString(u"<<END"), UnicodeString(u"foo"), UnicodeString(u"END"),
                       UnicodeString(u"int x")});
    auto here_editor = makeEditor(factory, here);
    REQUIRE(here_editor->getInvalidLine() == here.size());
    here.setLine(1, UnicodeString(u"foo2"));
    here_editor->modifyLineEvent(1);
    REQUIRE(here_editor->getInvalidLine() == here.size());
  }

  editor->removeRegionHandler(&counter);
}

TEST_CASE("Sequential getLineRegions colors the default window boundary lines", "[baseeditor]")
{
  ParserFactory factory;
  loadTryLine(factory);

  std::vector<UnicodeString> lines;
  lines.reserve(50);
  for (int i = 0; i < 50; i++) {
    lines.emplace_back(u"int a");
  }
  MutableLines source(std::move(lines));
  BaseEditor editor(&factory, &source);
  auto* type = factory.getHrcLibrary().getFileType(UnicodeString("try_line"));
  REQUIRE(type != nullptr);
  editor.setFileType(type);
  editor.lineCountEvent(source.size());
  // colorer -ht never calls visibleTextEvent; wSize stays at the ctor default 20.
  for (int i : {0, 20, 21, 40, 41}) {
    REQUIRE(hasRegion(editor.getLineRegions(i), "try_line:Kw"));
  }
}

TEST_CASE("A small window resize does not drop cached line regions", "[baseeditor]")
{
  ParserFactory factory;
  loadTryLine(factory);

  std::vector<UnicodeString> lines;
  lines.reserve(40);
  for (int i = 0; i < 40; i++) {
    lines.emplace_back(u"int a");
  }
  MutableLines source(std::move(lines));
  BaseEditor editor(&factory, &source);
  auto* type = factory.getHrcLibrary().getFileType(UnicodeString("try_line"));
  REQUIRE(type != nullptr);
  editor.setFileType(type);
  editor.lineCountEvent(source.size());
  editor.visibleTextEvent(0, 20);
  REQUIRE(hasRegion(editor.getLineRegions(0), "try_line:Kw"));
  REQUIRE(editor.getInvalidLine() >= 20);

  CountHandler counter;
  editor.addRegionHandler(&counter);
  editor.visibleTextEvent(0, 21);
  REQUIRE(hasRegion(editor.getLineRegions(0), "try_line:Kw"));
  REQUIRE(counter.clear_lines == 0);

  counter.clear_lines = 0;
  editor.visibleTextEvent(0, 50);
  REQUIRE(editor.getLineRegions(0) != nullptr);
  REQUIRE(counter.clear_lines == source.size());
  editor.removeRegionHandler(&counter);
}

TEST_CASE("idleJob warms the parse cache without moving visible line regions", "[baseeditor]")
{
  ParserFactory factory;
  loadTryLine(factory);

  std::vector<UnicodeString> lines;
  lines.reserve(80);
  for (int i = 0; i < 80; i++) {
    if (i == 0) {
      lines.emplace_back(u"int a");
    }
    else if (i == 60) {
      lines.emplace_back(u"foo");
    }
    else {
      lines.emplace_back(u"x");
    }
  }
  MutableLines source(std::move(lines));
  BaseEditor editor(&factory, &source);
  auto* type = factory.getHrcLibrary().getFileType(UnicodeString("try_line"));
  REQUIRE(type != nullptr);
  editor.setFileType(type);
  editor.lineCountEvent(source.size());
  editor.visibleTextEvent(0, 20);
  REQUIRE(hasRegion(editor.getLineRegions(0), "try_line:Kw"));

  CountHandler counter;
  editor.addRegionHandler(&counter);
  while (editor.haveInvalidLine()) {
    editor.idleJob(100);
  }
  REQUIRE(editor.getInvalidLine() == source.size());
  REQUIRE(hasRegion(editor.getLineRegions(0), "try_line:Kw"));

  editor.visibleTextEvent(50, 20);
  REQUIRE_FALSE(hasRegion(editor.getLineRegions(60), "try_line:Kw"));

  editor.visibleTextEvent(0, 20);
  REQUIRE(hasRegion(editor.getLineRegions(0), "try_line:Kw"));
  editor.removeRegionHandler(&counter);
}

TEST_CASE("Probe ParserFactory on the same thread does not leak types into master", "[baseeditor]")
{
  ParserFactory master;
  loadTryLine(master);

  {
    ParserFactory probe;
    auto cue_path = fs::path(__FILE__).parent_path() / "data" / "type_cue.hrc";
    UnicodeString cue_location(cue_path.c_str());
    probe.loadHrcPath(&cue_location);

    REQUIRE(probe.getHrcLibrary().getFileType(UnicodeString("cue")) != nullptr);
    REQUIRE(probe.getHrcLibrary().getFileType(UnicodeString("try_line")) == nullptr);
    REQUIRE(master.getHrcLibrary().getFileType(UnicodeString("cue")) == nullptr);
  }

  REQUIRE(master.getHrcLibrary().getFileType(UnicodeString("try_line")) != nullptr);
  REQUIRE(master.getHrcLibrary().getFileType(UnicodeString("cue")) == nullptr);

  MutableLines source({UnicodeString(u"int a")});
  auto editor = makeEditor(master, source);
  REQUIRE(hasRegion(editor->getLineRegions(0), "try_line:Kw"));
}

TEST_CASE("two editors parse one HrcLibrary on the same thread", "[baseeditor]")
{
  ParserFactory factory;
  loadTryLine(factory);

  MutableLines source_a({UnicodeString(u"int a")});
  MutableLines source_b({UnicodeString(u"int b")});
  auto editor_a = makeEditor(factory, source_a);
  auto editor_b = makeEditor(factory, source_b);

  for (int i = 0; i < 200; i++) {
    REQUIRE(hasRegion(editor_a->getLineRegions(0), "try_line:Kw"));
    REQUIRE(hasRegion(editor_b->getLineRegions(0), "try_line:Kw"));
  }
}

TEST_CASE("loading another type does not disturb an existing editor", "[baseeditor]")
{
  ParserFactory factory;
  loadTryLine(factory);

  MutableLines source({UnicodeString(u"int a")});
  auto editor = makeEditor(factory, source);
  REQUIRE(hasRegion(editor->getLineRegions(0), "try_line:Kw"));

  auto block_path = fs::path(__FILE__).parent_path() / "data" / "type_block.hrc";
  UnicodeString block_location(block_path.c_str());
  factory.loadHrcPath(&block_location);

  REQUIRE(hasRegion(editor->getLineRegions(0), "try_line:Kw"));
  REQUIRE(factory.getHrcLibrary().getFileType(UnicodeString("bl_quote")) != nullptr);
  REQUIRE(factory.getHrcLibrary().getFileType(UnicodeString("try_line")) != nullptr);
}

TEST_CASE("setFileType rejects a null type", "[baseeditor]")
{
  ParserFactory factory;
  MutableLines source({UnicodeString(u"x")});
  BaseEditor editor(&factory, &source);
  REQUIRE_THROWS_AS(editor.setFileType(static_cast<FileType*>(nullptr)), FileTypeException);
  REQUIRE_THROWS_AS(editor.setFileType(UnicodeString("no_such_type")), FileTypeException);
}
