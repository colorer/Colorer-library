#ifndef _COLORERTOOLS_VERSION_H_
#define _COLORERTOOLS_VERSION_H_

#define TOOLS_VER_MAJOR 1
#define TOOLS_VER_MINOR 0
#define TOOLS_VER_PATCH 4

#define TOOLS_COPYRIGHT "(c) 1999-2009 Igor Russkih, (c) 2009-2020 Aleksey Dobrunov"

#ifdef _WIN64
#define CONF " x64"
#elif defined _WIN32
#define CONF " x86"
#else
#define CONF ""
#endif

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define TOOLS_VERSION STRINGIZE(TOOLS_VER_MAJOR) "." STRINGIZE(TOOLS_VER_MINOR) "." STRINGIZE(TOOLS_VER_PATCH) CONF

#endif  // _COLORERTOOLS_VERSION_H_
