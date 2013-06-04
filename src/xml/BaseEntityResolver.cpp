#include <xml/BaseEntityResolver.h>
#include <xercesc/util/XMLString.hpp>
#include <xml/XmlInputSource.h>

xercesc::InputSource* BaseEntityResolver::resolveEntity(xercesc::XMLResourceIdentifier* resourceIdentifier)
{
  if(xercesc::XMLString::startsWith(resourceIdentifier->getBaseURI(), L"jar:") || xercesc::XMLString::findAny(resourceIdentifier->getSystemId(), L"%") )
  {
    return XmlInputSource::newInstance(resourceIdentifier->getSystemId(),resourceIdentifier->getBaseURI())->getInputSource(); 
  }
  return 0;
}