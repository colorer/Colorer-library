#include "tests.h"
#include "SanTimer.h"
#include<iostream>
using namespace std;

/* 
*  speed test a class ParserFactory constructor
*  search and reading catalog.xml
*/
void TestParserFactoryConstructor(int count, SString *catalogPath)
{
  cout<<"TestParserFactoryConstructor"<<endl;
  double all_time = 0, t;
  for (int i=0; i<=count; i++)
  {
    // start timer
    CTimer timer(true); 
    ParserFactory *parserFactoryLocal = NULL;
    parserFactoryLocal = new ParserFactory(catalogPath);
    // stop timer
    t = timer.StopGet();
    if (i)
    {
      all_time = all_time + t;
      cout<<t<<endl;
    }
    delete parserFactoryLocal;
  }
  cout<< "the average time for "<<count<<" tests "<<all_time/count << " sec."<<endl; 
}

/* 
*  speed test a ParserFactory->getHRCParser
*  load proto.hrc 
*/
void TestParserFactoryHRCParser(int count, SString *catalogPath)
{
  cout<<"TestParserFactoryHRCParser"<<endl;
  double all_time = 0, t;
  for (int i=0; i<=count; i++)
  {
    ParserFactory *parserFactoryLocal = NULL;
    parserFactoryLocal = new ParserFactory(catalogPath);
    // start timer
    CTimer timer(true);
    HRCParser *hrcParserLocal = NULL;
    hrcParserLocal = parserFactoryLocal->getHRCParser();
    // stop timer
    t = timer.StopGet();
    if (i)
    {
      all_time = all_time + t;
      cout<<t<<endl;
    }
    delete parserFactoryLocal;
  }
  cout<< "the average time for "<<count<<" tests "<<all_time/count << " sec."<<endl; 
}

/* 
*  speed test a ParserFactory->createStyledMapper
*  load default hrd scheme
*/
void TestParserFactoryStyledMapper(int count, SString *catalogPath)
{
  cout<<"TestParserFactoryStyledMapper"<<endl;
  double all_time = 0, t;
  for (int i=0; i<=count; i++)
  {
    ParserFactory *parserFactoryLocal = NULL;
    parserFactoryLocal = new ParserFactory(catalogPath);
    // start timer
    CTimer timer(true);
    RegionMapper *regionMapperLocal = NULL;
    regionMapperLocal = parserFactoryLocal->createStyledMapper(&DString("console"), NULL);

    // stop timer
    t = timer.StopGet();
    if (i)
    {
      all_time = all_time + t;
      cout<<t<<endl;
    }
    delete regionMapperLocal;
    delete parserFactoryLocal;
  }
  cout<< "the average time for "<<count<<" tests "<<all_time/count << " sec."<<endl; 
}

/* 
*  speed test load all hrc sheme
*/
void TestParserFactoryLoadAllHRCScheme(int count, SString *catalogPath)
{
  cout<<"TestParserFactoryLoadAllHRCScheme"<<endl;
  double all_time = 0, t;
  for (int i=0; i<=count; i++)
  {
    ParserFactory *parserFactoryLocal = NULL;
    parserFactoryLocal = new ParserFactory(catalogPath);
    HRCParser *hrcParserLocal = NULL;
    hrcParserLocal = parserFactoryLocal->getHRCParser();
    // start timer
    CTimer timer(true);

    for (int idx = 0;; idx++)
    {
      FileType *type = hrcParserLocal->enumerateFileTypes(idx);
      if (type == NULL) break;
      type->getBaseScheme();
    }
    // stop timer
    t = timer.StopGet();
    if (i)
    {
      all_time = all_time + t;
      cout<<t<<endl;
    }
    delete parserFactoryLocal;
  }
  cout<< "the average time for "<<count<<" tests "<<all_time/count << " sec."<<endl; 
}


FileType *selectType(HRCParser *hrcParser, LineSource *lineSource, SString *testFile){
  FileType *type = null;

  StringBuffer textStart;
  int totalLength = 0;
  for(int i = 0; i < 4; i++){
    String *iLine = lineSource->getLine(i);
    if (iLine == null) break;
    textStart.append(iLine);
    textStart.append(DString("\n"));
    totalLength += iLine->length();
    if (totalLength > 500) break;
  }
  type = hrcParser->chooseFileType(testFile, &textStart, 0);

  return type;
}

/* 
*  speed test coloring file
*/
void TestColoringFile(int count, SString *catalogPath, SString *testFile)
{
  cout<<"TestColoringFile"<<endl;
  double all_time = 0, t;
  for (int i=0; i<=count; i++)
  {
    ParserFactory *parserFactoryLocal = NULL;
    parserFactoryLocal = new ParserFactory(catalogPath);
    // Source file text lines store.
    TextLinesStore textLinesStore;
    textLinesStore.loadFile(testFile, NULL, true);
    // Base editor to make primary parse
    BaseEditor baseEditor(parserFactoryLocal, &textLinesStore);
    // HRD RegionMapper linking
    baseEditor.setRegionMapper(&DString("console"), NULL);
    FileType *type = selectType(parserFactoryLocal->getHRCParser(), &textLinesStore,testFile);
    type->getBaseScheme();
    baseEditor.setFileType(type);

    // start timer
    CTimer timer(true);

    baseEditor.modifyLineEvent(0);
    baseEditor.lineCountEvent(textLinesStore.getLineCount());
    baseEditor.validate(-1, false);

    // stop timer
    t = timer.StopGet();
    if (i)
    {
      all_time = all_time + t;
      cout<<t<<endl;
    }
    delete parserFactoryLocal;
  }
  cout<< "the average time for "<<count<<" tests "<<all_time/count << " sec."<<endl; 
}