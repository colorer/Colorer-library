#include <colorer/ParserFactory.h>
#include <colorer/parsers/ParserFactoryImpl.h>

ParserFactory::ParserFactory() : pimpl(spimpl::make_unique_impl<Impl>()) {}

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

std::vector<const HRDNode*> ParserFactory::enumHRDInstances(const UnicodeString& classID)
{
  return pimpl->enumHRDInstances(classID);
}

void ParserFactory::addHrd(std::unique_ptr<HRDNode> hrd)
{
  pimpl->addHrd(std::move(hrd));
}

const HRDNode* ParserFactory::getHRDNode(const UnicodeString& classID, const UnicodeString& nameID)
{
  return pimpl->getHRDNode(classID, nameID);
}

void ParserFactory::loadHrcPath(const UnicodeString& location)
{
  pimpl->loadHrcPath(location);
}