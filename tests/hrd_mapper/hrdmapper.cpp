#include <colorer/common/UStr.h>
#include <colorer/handlers/StyledHRDMapper.h>
#include <colorer/handlers/TextHRDMapper.h>
#include <colorer/io/StreamWriter.h>
#include <colorer/xml/XmlInputSource.h>
#include <iostream>

void testTextHrd()
{
  StreamWriter writer(stdout, -1, false);

  UnicodeString file = R"(./data/tags.hrd)";

  auto dfis = XmlInputSource::newInstance(UStr::to_xmlch(&file).get(), (XMLCh*) nullptr);

  TextHRDMapper thrd;
  thrd.loadRegionMappings(dfis.get());

  auto region = thrd.getRegionDefine("def:Date");
  auto t_region = TextRegion::cast(region);
  std::cout << "start=" << UStr::to_stdstr(t_region->start_text.get()) << " end=" << UStr::to_stdstr(t_region->end_text.get()) << "\n";

  thrd.saveRegionMappings(&writer);
}

void testStyledHrd()
{
  StreamWriter writer(stdout, -1, false);

  UnicodeString file = R"(./data/blue.hrd)";

  auto dfis = XmlInputSource::newInstance(UStr::to_xmlch(&file).get(), (XMLCh*) nullptr);

  StyledHRDMapper shrd;
  shrd.loadRegionMappings(dfis.get());

  auto region = shrd.getRegionDefine("def:PairEnd");
  auto s_region = StyledRegion::cast(region);
  std::cout << "fore=" << s_region->fore << " back=" << s_region->back << " style=" << s_region->style << "\n";

  shrd.saveRegionMappings(&writer);
}

int main(int /*argc*/, char** /*argv[]*/)
{
  xercesc::XMLPlatformUtils::Initialize();

  testTextHrd();
  testStyledHrd();

  xercesc::XMLPlatformUtils::Terminate();
  return 0;
}