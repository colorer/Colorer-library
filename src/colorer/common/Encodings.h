#ifndef COLORER_COMMON_ENCODINGS_H
#define COLORER_COMMON_ENCODINGS_H

#include "colorer/Common.h"

constexpr char ENC_UTF8[] = "UTF-8";

class Encodings
{
 public:
  static uUnicodeString toUnicodeString(char* data, int32_t len);
};

#endif  // COLORER_COMMON_ENCODINGS_H
