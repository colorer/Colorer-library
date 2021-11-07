#ifndef COLORER_COMMON_H
#define COLORER_COMMON_H

#include <spdlog/spdlog.h>
#include "colorer/common/Features.h"
#include "colorer/common/UnicodeLogger.h"
#include "colorer/common/UnicodeStringContainer.h"

/// system dependent byte
// TODO std::byte
using byte = unsigned char;
using UnicodeString = icu::UnicodeString;
using uUnicodeString = std::unique_ptr<UnicodeString>;

constexpr UChar BAD_WCHAR = 0xFFFF;

#ifdef COLORER_USE_DEEPTRACE
#define CTRACE(info) info
#else
#define CTRACE(info)
#endif

#endif  // COLORER_COMMON_H
