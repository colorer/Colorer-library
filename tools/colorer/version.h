#define VER_FILEVERSION             1,0,0,0
#define VER_FILEVERSION_STR         "1.0.0.0"

#define VER_PRODUCTVERSION          VER_FILEVERSION
#define VER_PRODUCTVERSION_STR      VER_FILEVERSION_STR
#ifdef _WIN64
  #define CONF " (x64)"
#else
  #define CONF ""
#endif
#define FILE_DESCRIPTION "Colorer - Syntax Highlighting tools" CONF
