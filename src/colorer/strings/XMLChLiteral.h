#ifndef COLORER_XMLCHLITERAL_H
#define COLORER_XMLCHLITERAL_H

#include <xercesc/util/Xerces_autoconf_config.hpp>

// crutch for a case when Xerces has XMLCh be alias for uint16_t instead of char16_t
// so far, it has only been found on macOS

static_assert(sizeof(char16_t) == sizeof(XMLCh), "XMLCh must be 16-bit type");

#define XMLCH_LITERAL(name, value) inline const auto name = (const XMLCh *) value;
#define XMLCH_LITERAL_LOCAL(name, value) const auto name = (const XMLCh *) value;
#define XMLCH_ARRAY(name, value) XMLCh name[sizeof(value) / sizeof(char16_t)]; memcpy(name, value, sizeof(name));

#define UNICODE_LITERAL(name, value) inline const auto name = UnicodeString(value);

#endif  // COLORER_XMLCHLITERAL_H
