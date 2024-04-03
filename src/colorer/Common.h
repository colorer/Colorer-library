#ifndef COLORER_COMMON_H
#define COLORER_COMMON_H

#include "colorer/common/Features.h"
#include "colorer/common/UnicodeStringContainer.h"
#include <unicode/uniset.h>

/// system dependent byte
// TODO std::byte
using byte = unsigned char;
using UnicodeString = icu::UnicodeString;
using uUnicodeString = std::unique_ptr<UnicodeString>;
using CharacterClass = icu::UnicodeSet;

constexpr UChar BAD_WCHAR = 0xFFFF;

/*
 Logging
*/

#ifdef COLORER_USE_DEEPTRACE
#define CTRACE(info) info
#else
#define CTRACE(info)
#endif

#ifndef COLORER_FEATURE_DUMMYLOGGER

#include <spdlog/spdlog.h>
#include "colorer/common/UnicodeLogger.h"

extern std::shared_ptr<spdlog::logger> logger;

#else // COLORER_FEATURE_DUMMYLOGGER
class DummyLogger
{
 public:
  template <typename... Args>
  void debug(Args... /*args*/)
  {
  }
  template <typename... Args>
  void error(Args... /*args*/)
  {
  }
  template <typename... Args>
  void warn(Args... /*args*/)
  {
  }
  template <typename... Args>
  void trace(Args... /*args*/)
  {
  }
};
extern std::shared_ptr<DummyLogger> logger;
#endif // COLORER_FEATURE_DUMMYLOGGER

#endif  // COLORER_COMMON_H
