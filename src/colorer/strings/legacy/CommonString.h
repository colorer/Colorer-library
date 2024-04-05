#ifndef _COLORER_COMMONSTRING_H_
#define _COLORER_COMMONSTRING_H_

#include <wchar.h>
#include <xercesc/util/XMLChar.hpp>

/// default unicode char definition

typedef wchar_t UChar;
typedef wchar_t wchar;
typedef char16_t w2char;
typedef char32_t w4char;
typedef unsigned char byte;

#define BAD_WCHAR ((wchar)0xFFFF)

#endif // _COLORER_COMMONSTRING_H_


