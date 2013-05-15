#include<xml/xmldom.h>
#include<common/io/FileInputSource.h>
#include"xmlprint.cpp"

int main(int argc, char *argv[])
{
  FileInputSource fis = FileInputSource(&DString(argv[1]), null);
/*
  hFile = CreateFile(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (hFile == INVALID_HANDLE_VALUE){
    printf("can't read: %s\n", argv[1]);
    return 0;
  };
  size = GetFileSize(hFile, NULL);
  byte *bytes = new byte[size];
  ReadFile(hFile, bytes, size, &res, NULL);
*/
  printf("FILE:\t%s\t\t\t\t", argv[1]);
  try{

    int t1 = GetTickCount();

    DocumentBuilder *db = new DocumentBuilder();

    Document *doc = db->parse(&fis);

    int t2 = GetTickCount()-t1;

    printf("TIMING:\t%d\n", t2);

    printLevel(doc, 0);

    db->free(doc);
    delete db;
  }catch(Exception &e){
    printf("\n\nEXC: %s\n\n", e.getMessage()->getChars());
  }catch(...){
    printf("UNK\n");
  }

/*
  int sz = parser->getLevelSize(0);
  char *serialized = new char[sz+1];
  parser->saveLevel(serialized, 0);
  serialized[sz]=0;
  printf(serialized);
  delete[] serialized;
*/

/*
  char *chars;
  parser->next()->next()->next()->next()->next()->getName()->getBytes(&chars, "cp1251");
  printf("%s\n", chars);
  delete[] chars;
*/

  return 0;
};
