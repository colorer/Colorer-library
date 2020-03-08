#include <colorer/parsers/ParserFactoryImpl.h>

ParserFactory::ParserFactory() : pimpl(new ParserFactory::Impl()) {}

void ParserFactory::loadCatalog(const UnicodeString* catalog_path)
{
  pimpl->loadCatalog(catalog_path);
}

HRCParser* ParserFactory::getHRCParser() const
{
  return pimpl->getHRCParser();
}

TextParser* ParserFactory::createTextParser()
{
  return pimpl->createTextParser();
}

StyledHRDMapper* ParserFactory::createStyledMapper(const UnicodeString* classID, const UnicodeString* nameID)
{
  return pimpl->createStyledMapper(classID, nameID);
}

TextHRDMapper* ParserFactory::createTextMapper(const UnicodeString* nameID)
{
  return pimpl->createTextMapper(nameID);
}
