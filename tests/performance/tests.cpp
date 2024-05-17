#include "tests.h"
#include <iostream>
#include <chrono>
using namespace std;
using namespace std::chrono;

/*
 *  speed test a class ParserFactory constructor
 *  search and reading catalog.xml
 */
void TestParserFactoryConstructor(int count, UnicodeString* catalogPath)
{
  cout << "TestParserFactoryConstructor" << endl;
  double all_time = 0;
  for (int i = 0; i <= count; i++) {
    // start timer
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    ParserFactory parserFactoryLocal;
    parserFactoryLocal.loadCatalog(catalogPath);
    // stop timer
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto time_span = duration_cast<duration<double>>(t2 - t1);

    if (i) {
      all_time = all_time + time_span.count();
      cout << time_span.count() << endl;
    }
  }
  cout << "the average time for " << count << " tests " << all_time / count << " sec." << endl;
}

/*
 *  speed test a ParserFactory->getHrcLibrary
 *  load proto.hrc
 */
void TestParserFactoryHrcLibrary(int count, UnicodeString* catalogPath)
{
  cout << "TestParserFactoryHrcLibrary" << endl;
  double all_time = 0;
  for (int i = 0; i <= count; i++) {
    ParserFactory parserFactoryLocal;
    parserFactoryLocal.loadCatalog(catalogPath);
    // start timer
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    auto& hrcLibraryLocal = parserFactoryLocal.getHrcLibrary();
    // stop timer
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto time_span = duration_cast<duration<double>>(t2 - t1);

    if (i) {
      all_time = all_time + time_span.count();
      cout << time_span.count() << endl;
    }
  }
  cout << "the average time for " << count << " tests " << all_time / count << " sec." << endl;
}

/*
 *  speed test a ParserFactory->createStyledMapper
 *  load default hrd scheme
 */
void TestParserFactoryStyledMapper(int count, UnicodeString* catalogPath)
{
  cout << "TestParserFactoryStyledMapper" << endl;
  double all_time = 0;
  for (int i = 0; i <= count; i++) {
    ParserFactory parserFactoryLocal;
    parserFactoryLocal.loadCatalog(catalogPath);
    // start timer
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    auto console = UnicodeString("console");
    auto regionMapperLocal = parserFactoryLocal.createStyledMapper(&console, nullptr);

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto time_span = duration_cast<duration<double>>(t2 - t1);

    if (i) {
      all_time = all_time + time_span.count();
      cout << time_span.count() << endl;
    }
  }
  cout << "the average time for " << count << " tests " << all_time / count << " sec." << endl;
}

/*
 *  speed test load all hrc sheme
 */
void TestParserFactoryLoadAllHRCScheme(int count, UnicodeString* catalogPath)
{
  cout << "TestParserFactoryLoadAllHRCScheme" << endl;
  double all_time = 0;
  for (int i = 0; i <= count; i++) {
    ParserFactory parserFactoryLocal;
    parserFactoryLocal.loadCatalog(catalogPath);
    auto& hrcLibraryLocal = parserFactoryLocal.getHrcLibrary();
    // start timer
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    for (int idx = 0;; idx++) {
      FileType* type = hrcLibraryLocal.enumerateFileTypes(idx);
      if (type == nullptr)
        break;
      type->getBaseScheme();
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto time_span = duration_cast<duration<double>>(t2 - t1);

    if (i) {
      all_time = all_time + time_span.count();
      cout << time_span.count() << endl;
    }
  }
  cout << "the average time for " << count << " tests " << all_time / count << " sec." << endl;
}

FileType* selectType(HrcLibrary* hrcLibrary, LineSource* lineSource, UnicodeString* testFile)
{
  FileType* type;

  UnicodeString textStart;
  int totalLength = 0;
  for (int i = 0; i < 4; i++) {
    UnicodeString* iLine = lineSource->getLine(i);
    if (iLine == nullptr)
      break;
    textStart.append(*iLine);
    textStart.append(UnicodeString("\n"));
    totalLength += iLine->length();
    if (totalLength > 500)
      break;
  }
  type = hrcLibrary->chooseFileType(testFile, &textStart, 0);

  return type;
}

/*
 *  speed test coloring file
 */
void TestColoringFile(int count, UnicodeString* catalogPath, UnicodeString* testFile)
{
  cout << "TestColoringFile" << endl;
  double all_time = 0;
  for (int i = 0; i <= count; i++) {
    ParserFactory parserFactoryLocal;
    parserFactoryLocal.loadCatalog(catalogPath);
    // Source file text lines store.
    TextLinesStore textLinesStore;
    textLinesStore.loadFile(testFile, true);
    // Base editor to make primary parse
    BaseEditor baseEditor(&parserFactoryLocal, &textLinesStore);
    // HRD RegionMapper linking
    auto console = UnicodeString("console");
    baseEditor.setRegionMapper(&console, nullptr);
    FileType* type = selectType(&parserFactoryLocal.getHrcLibrary(), &textLinesStore, testFile);
    type->getBaseScheme();
    baseEditor.setFileType(type);

    // start timer
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    baseEditor.modifyLineEvent(0);
    baseEditor.lineCountEvent((int) textLinesStore.getLineCount());
    baseEditor.validate(-1, false);

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto time_span = duration_cast<duration<double>>(t2 - t1);

    if (i) {
      all_time = all_time + time_span.count();
      cout << time_span.count() << endl;
    }
  }
  cout << "the average time for " << count << " tests " << all_time / count << " sec." << endl;
}