#include <cwchar>
#include <memory>
#include "tests.h"

enum JobType { JT_NOTHING, JT_TEST1, JT_TEST2, JT_TEST3, JT_TEST4, JT_TEST5 };

int loops = 1;
JobType job = JT_NOTHING;
UnicodeString* catalogPath = nullptr;
UnicodeString* testFile = nullptr;

void printError()
{
  fwprintf(stderr,
           L"\nUsage: speed_test (command) (parameters)\n"
           L" Commands:\n"
           L"  -t<n>      Run the test number <n>\n"
           L" Parameters:\n"
           L"  -c<n>      Number of test runs\n"
           L"  -b<path>   Uses specified 'catalog.xml' file\n"
           L"  -f<path>   Test file\n\n"
           L" Test:\n"
           L"   1         TestParserFactoryConstructor\n"
           L"   2         TestParserFactoryHRCParser\n"
           L"   3         TestParserFactoryStyledMapper\n"
           L"   4         TestParserFactoryLoadAllHRCScheme\n"
           L"   5         TestColoringFile\n");
}

int init(int argc, char* argv[])
{
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != L'-') {
      return -1;
    }
    if (argv[i][1] == L't') {
      if (argv[i][2]) {
        job = (JobType) atoi(argv[i] + 2);
      } else
        return -1;
      continue;
    }
    if (argv[i][1] == L'c') {
      if (argv[i][2]) {
        loops = atoi(argv[i] + 2);
      }
      if (!loops)
        loops = 1;
      continue;
    }
    if (argv[i][1] == L'b' && (i + 1 < argc || argv[i][2])) {
      if (argv[i][2]) {
        catalogPath = new UnicodeString(argv[i] + 2);
      } else {
        catalogPath = new UnicodeString(argv[i + 1]);
        i++;
      }
      continue;
    }
    if (argv[i][1] == L'f' && (i + 1 < argc || argv[i][2])) {
      if (argv[i][2]) {
        testFile = new UnicodeString(argv[i] + 2);
      } else {
        testFile = new UnicodeString(argv[i + 1]);
        i++;
      }
      continue;
    }
    if (argv[i][1]) {
      fprintf(stderr, "WARNING: unknown option '-%s'\n", argv[i] + 1);
      return -1;
    }
  }
  return 1;
}

int main(int argc, char* argv[])
{
  if ((argc < 2) || (init(argc, argv) == -1)) {
    printError();
    return 1;
  }

  try {
    switch (job) {
      case JT_NOTHING:
        printError();
        break;
      case JT_TEST1:
        TestParserFactoryConstructor(loops, catalogPath);
        break;
      case JT_TEST2:
        TestParserFactoryHRCParser(loops, catalogPath);
        break;
      case JT_TEST3:
        TestParserFactoryStyledMapper(loops, catalogPath);
        break;
      case JT_TEST4:
        TestParserFactoryLoadAllHRCScheme(loops, catalogPath);
        break;
      case JT_TEST5:
        TestColoringFile(loops, catalogPath, testFile);
        break;
    }
  } catch (Exception& e) {
    fprintf(stderr, "%s\n", e.what());
    return -1;
  }
  return 0;
}
