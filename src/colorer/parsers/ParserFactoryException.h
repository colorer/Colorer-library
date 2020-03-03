#ifndef _COLORER_PARSERFACTORY_EXCEPTION_H_
#define _COLORER_PARSERFACTORY_EXCEPTION_H_

#include<colorer/Common.h>

/** Exception, thrown by ParserFactory class methods.
    Indicates some (mostly fatal) errors in loading
    of catalog file (catalog.xml), or in creating
    parsers objects.
    @ingroup colorer
*/
class ParserFactoryException : public Exception
{
 public:
  explicit ParserFactoryException(const UnicodeString& msg) noexcept : Exception("[ParserFactoryException] " + msg) {}
};

#endif


