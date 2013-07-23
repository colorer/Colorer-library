#include <xml/BaseEntityResolver.h>
#include <xercesc/util/XMLString.hpp>
#include <xml/XmlInputSource.h>

xercesc::InputSource* BaseEntityResolver::resolveEntity(xercesc::XMLResourceIdentifier* resourceIdentifier)
{
  if(xercesc::XMLString::startsWith(resourceIdentifier->getBaseURI(), kJar) || xercesc::XMLString::findAny(resourceIdentifier->getSystemId(), kPercent) )
  {
    return XmlInputSource::newInstance(resourceIdentifier->getSystemId(),resourceIdentifier->getBaseURI())->getInputSource(); 
  }
  return 0;
}