#include "colorer/Common.h"
#include "colorer/common/UStr.h"
#include "colorer/xml/XmlParserErrorHandler.h"

void XmlParserErrorHandler::warning(const xercesc::SAXParseException& toCatch)
{
  spdlog::warn("Warning at file {0}, line {1}, column {2}. Message: {3}", UStr::to_stdstr(toCatch.getSystemId()), toCatch.getLineNumber(),
               toCatch.getColumnNumber(), UStr::to_stdstr(toCatch.getMessage()));
}

void XmlParserErrorHandler::error(const xercesc::SAXParseException& toCatch)
{
  fSawErrors = true;
  spdlog::error("Error at file {0}, line {1}, column {2}. Message: {3}", UStr::to_stdstr(toCatch.getSystemId()), toCatch.getLineNumber(),
                toCatch.getColumnNumber(), UStr::to_stdstr(toCatch.getMessage()));
}

void XmlParserErrorHandler::fatalError(const xercesc::SAXParseException& toCatch)
{
  fSawErrors = true;
  spdlog::error("Fatal error at file {0}, line {1}, column {2}. Message: {3}", UStr::to_stdstr(toCatch.getSystemId()), toCatch.getLineNumber(),
                toCatch.getColumnNumber(), UStr::to_stdstr(toCatch.getMessage()));
}
