#include <colorer/parsers/ParserFactoryImpl.h>
#include <colorer/editor/BaseEditor.h>
#include <colorer/viewer/TextLinesStore.h>

void TestParserFactoryConstructor(int count, UnicodeString* catalogPath);
void TestParserFactoryHRCParser(int count, UnicodeString* catalogPath);
void TestParserFactoryStyledMapper(int count, UnicodeString* catalogPath);
void TestParserFactoryLoadAllHRCScheme(int count, UnicodeString* catalogPath);
void TestColoringFile(int count, UnicodeString* catalogPath, UnicodeString* testFile);