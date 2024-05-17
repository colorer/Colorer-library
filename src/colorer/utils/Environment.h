#ifndef COLORER_ENVIRONMENT_H
#define COLORER_ENVIRONMENT_H

#include <filesystem>
namespace fs = std::filesystem;
#include "colorer/Common.h"

class Environment
{
 public:
  static uUnicodeString getOSVariable(const UnicodeString& name);
  static uUnicodeString expandEnvironment(const UnicodeString* path);
  static uUnicodeString normalizePath(const UnicodeString* path);
  static fs::path normalizeFsPath(const UnicodeString* path);
  static fs::path getClearFilePath(const UnicodeString* basePath, const UnicodeString* relPath);
  static fs::path to_filepath(const UnicodeString* str);

  static std::vector<UnicodeString> getFilesFromPath(const UnicodeString* basePath, const UnicodeString* relPath,
                                                     const UnicodeString& extension);
  static bool isRegularFile(const UnicodeString* basePath, const UnicodeString* relPath, UnicodeString& fullPath);
};

#endif  // COLORER_ENVIRONMENT_H
