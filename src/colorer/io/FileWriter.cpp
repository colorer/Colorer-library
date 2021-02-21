#include <colorer/common/UStr.h>
#include <colorer/io/FileWriter.h>
#include <cstdio>

FileWriter::FileWriter(const UnicodeString* fileName, bool useBOM)
{
  file = fopen(UStr::to_stdstr(fileName).c_str(), "wb+");
  init(file, useBOM);
}

FileWriter::FileWriter(const UnicodeString* fileName)
{
  file = fopen(UStr::to_stdstr(fileName).c_str(), "wb+");
  init(file, false);
}

FileWriter::~FileWriter()
{
  fclose(file);
}
