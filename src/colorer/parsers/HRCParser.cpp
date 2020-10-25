#include <colorer/HRCParser.h>
#include <colorer/parsers/HRCParserImpl.h>

HRCParser::HRCParser() : pimpl(spimpl::make_unique_impl<Impl>()) {}

void HRCParser::loadSource(XmlInputSource* is)
{
  pimpl->loadSource(is);
}

FileType* HRCParser::enumerateFileTypes(unsigned int index)
{
  return pimpl->enumerateFileTypes(index);
}

FileType* HRCParser::getFileType(const UnicodeString* name)
{
  return pimpl->getFileType(name);
}

FileType* HRCParser::chooseFileType(const UnicodeString* fileName, const UnicodeString* firstLine, int typeNo)
{
  return pimpl->chooseFileType(fileName, firstLine, typeNo);
}

size_t HRCParser::getFileTypesCount()
{
  return pimpl->getFileTypesCount();
}

size_t HRCParser::getRegionCount()
{
  return pimpl->getRegionCount();
}

const Region* HRCParser::getRegion(unsigned int id)
{
  return pimpl->getRegion(id);
}

const Region* HRCParser::getRegion(const UnicodeString* name)
{
  return pimpl->getRegion(name);
}

void HRCParser::loadFileType(FileType* filetype)
{
  pimpl->loadFileType(filetype);
}
