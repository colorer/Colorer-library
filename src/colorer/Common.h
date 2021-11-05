#ifndef _COLORER_COMMON_H_
#define _COLORER_COMMON_H_

/// system dependent byte
typedef unsigned char byte;

#include "colorer/common/UnicodeStringContainer.h"
#include <unicode/unistr.h>
#include <memory>
typedef icu::UnicodeString UnicodeString;
typedef std::unique_ptr<UnicodeString> uUnicodeString;

#include "colorer/common/UnicodeLogger.h"
#include <spdlog/spdlog.h>
#include "colorer/common/Features.h"
#define BAD_WCHAR (0xFFFF)

#ifdef COLORER_USE_DEEPTRACE
#define CTRACE(info) info
#else
#define CTRACE(info)
#endif

#endif
