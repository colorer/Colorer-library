#include "XmlParserErrorHandler.h"
#include <xml/XStr.h>


void XmlParserErrorHandler::warning(const xercesc::SAXParseException& toCatch)
{
  LOG(WARNING) << "Warning at file " << XStr(toCatch.getSystemId()) << ", line " << toCatch.getLineNumber() <<
    ", column " << toCatch.getColumnNumber() << " Message: "<< XStr(toCatch.getMessage());
}

void XmlParserErrorHandler::error(const xercesc::SAXParseException& toCatch)
{
  fSawErrors = true;
  LOG(ERROR) << "Error at file " << XStr(toCatch.getSystemId()) << ", line " << toCatch.getLineNumber() <<
    ", column " << toCatch.getColumnNumber() << " Message: " << XStr(toCatch.getMessage());
}

void XmlParserErrorHandler::fatalError(const xercesc::SAXParseException& toCatch)
{
  fSawErrors = true;
  LOG(ERRORF) << "Fatal Error at file " << XStr(toCatch.getSystemId()) << ", line " << toCatch.getLineNumber() <<
    ", column " << toCatch.getColumnNumber() << " Message: " << XStr(toCatch.getMessage());
}