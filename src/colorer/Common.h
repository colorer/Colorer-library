#ifndef _COLORER_COMMON_H_
#define _COLORER_COMMON_H_

/// system dependent byte
typedef unsigned char byte;

#include <colorer/common/UnicodeStringContainer.h>
#include <unicode/unistr.h>
#include <memory>
typedef icu::UnicodeString UnicodeString;
typedef std::unique_ptr<UnicodeString> uUnicodeString;

#include <colorer/common/Features.h>
#include <colorer/common/UnicodeLogger.h>
#include <spdlog/spdlog.h>

#endif
