#include <colorer/Exception.h>
#include <colorer/io/StreamWriter.h>
#include <unicode/ustring.h>
#include <cstdio>

StreamWriter::StreamWriter()= default;

void StreamWriter::init(FILE *fstream, int encoding, bool useBOM){
  
  if (fstream == nullptr) throw Exception("Invalid stream");
  file = fstream;
//  if (encoding == -1) encoding = Encodings::getDefaultEncodingIndex();
  encodingIndex = 4;
  this->useBOM = useBOM;
  writeBOM();
}

void StreamWriter::writeBOM(){
  //if (useBOM && Encodings::isMultibyteEncoding(encodingIndex))
  if (useBOM) write(0xFEFF);
}

StreamWriter::StreamWriter(FILE *fstream, int encoding = -1, bool useBOM = true){
  init(fstream, encoding, useBOM);
}

StreamWriter::~StreamWriter()= default;

void StreamWriter::write(UChar c){
  char buf[8];
  int32_t len = 0;
  UErrorCode err = U_ZERO_ERROR;
  u_strToUTF8(buf, 8, &len, &c, 1, &err);
  //TODO encoding
  //int bufLen = Encodings::toBytes(encodingIndex, c, buf);
  for(int pos = 0; pos < len; pos++)
    putc(buf[pos], file);
}


