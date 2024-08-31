#include <colorer/handlers/StyledHRDMapper.h>
#include <colorer/handlers/TextHRDMapper.h>
#include <colorer/io/StreamWriter.h>
#include <iostream>

void testTextHrd()
{
  StreamWriter writer(stdout, false);

  UnicodeString file = R"(./data/tags.hrd)";

  XmlInputSource dfis(file);

  TextHRDMapper thrd;
  thrd.loadRegionMappings(dfis);

  auto region = thrd.getRegionDefine("def:Date");
  auto t_region = TextRegion::cast(region);
  std::cout << "start=" << UStr::to_stdstr(t_region->start_text.get())
            << " end=" << UStr::to_stdstr(t_region->end_text.get()) << "\n";

  thrd.saveRegionMappings(&writer);
}

void testStyledHrd()
{
  StreamWriter writer(stdout, false);

  UnicodeString file = R"(./data/blue.hrd)";

  XmlInputSource dfis(file);

  StyledHRDMapper shrd;
  shrd.loadRegionMappings(dfis);

  auto region = shrd.getRegionDefine("def:PairEnd");
  auto s_region = StyledRegion::cast(region);
  std::cout << "fore=" << s_region->fore << " back=" << s_region->back
            << " style=" << s_region->style << "\n";

  shrd.saveRegionMappings(&writer);
}

int main(int /*argc*/, char** /*argv[]*/)
{
  testTextHrd();
  testStyledHrd();

  return 0;
}