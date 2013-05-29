#include "XmlParserErrorHandler.h"

void XmlParserErrorHandler::warning(const xercesc::SAXParseException& toCatch)
{
  if (error_handler) {
    error_handler->warning(StringBuffer("Warning at file ")+DString(toCatch.getSystemId())+DString(", line ")+
      SString(toCatch.getLineNumber())+DString(", column ")+SString(toCatch.getColumnNumber())+DString(" Message: ")+DString(toCatch.getMessage()));
  }
}

void XmlParserErrorHandler::error(const xercesc::SAXParseException& toCatch)
{
  fSawErrors = true;
  if (error_handler) {
    error_handler->error(StringBuffer("Error at file ")+DString(toCatch.getSystemId())+DString(", line ")+
      SString(toCatch.getLineNumber())+DString(", column ")+SString(toCatch.getColumnNumber())+DString(" Message: ")+DString(toCatch.getMessage()));
  }
}

void XmlParserErrorHandler::fatalError(const xercesc::SAXParseException& toCatch)
{
  fSawErrors = true;
  if (error_handler) {
    error_handler->fatalError(StringBuffer("Fatal Error at file ")+DString(toCatch.getSystemId())+DString(", line ")+
      SString(toCatch.getLineNumber())+DString(", column ")+SString(toCatch.getColumnNumber())+DString(" Message: ")+DString(toCatch.getMessage()));
  }
}