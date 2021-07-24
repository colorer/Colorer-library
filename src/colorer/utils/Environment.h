#ifndef COLORER_ENVIRONMENT_H
#define COLORER_ENVIRONMENT_H

#include <colorer/Common.h>

class Environment
{
 public:
  static uUnicodeString getOSVariable(const char* name);
  static uUnicodeString expandEnvironment(const UnicodeString* path);
};

#endif  // COLORER_ENVIRONMENT_H
