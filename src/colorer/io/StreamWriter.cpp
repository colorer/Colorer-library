#include <colorer/Exception.h>
#include <colorer/io/StreamWriter.h>
#include <unicode/ustring.h>
#include <cstdio>

void StreamWriter::init(FILE* fstream, bool _useBOM)
{
  if (fstream == nullptr)
    throw Exception("Invalid stream");
  file = fstream;
  useBOM = _useBOM;
  writeBOM();
}

void StreamWriter::writeBOM()
{
  if (useBOM) {
    putc(0xEF, file);
    putc(0xBB, file);
    putc(0xBF, file);
  }
}

StreamWriter::StreamWriter(FILE* fstream, bool _useBOM = true)
{
  init(fstream, _useBOM);
}

void StreamWriter::write(UChar c)
{
  char buf[8];
  int32_t len = 0;
  UErrorCode err = U_ZERO_ERROR;
  u_strToUTF8(buf, 8, &len, &c, 1, &err);
  for (int pos = 0; pos < len; pos++) putc(buf[pos], file);
}
