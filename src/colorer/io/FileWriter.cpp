
#include<cstdio>
#include <colorer/common/UStr.h>
#include<colorer/io/FileWriter.h>

FileWriter::FileWriter(const UnicodeString *fileName, int encoding, bool useBOM){
  file = fopen(UStr::to_stdstr(fileName).c_str(), "wb+");
  init(file, encoding, useBOM);
}
FileWriter::FileWriter(const UnicodeString *fileName){
  file = fopen(UStr::to_stdstr(fileName).c_str(), "wb+");
  init(file, -1, false);
}
FileWriter::~FileWriter(){
  fclose(file);
}



