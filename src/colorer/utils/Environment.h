#ifndef COLORER_ENVIRONMENT_H
#define COLORER_ENVIRONMENT_H

#include <colorer/Common.h>

class Environment
{
 public:
  static uUnicodeString getOSVariable(const char* name);
};

#endif  // COLORER_ENVIRONMENT_H
