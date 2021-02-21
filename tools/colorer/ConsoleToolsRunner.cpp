#include <cstdio>
#include <cstdlib>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "ConsoleTools.h"
#include <colorer/version.h>

/** Internal run action type */
enum JobType { JT_NOTHING, JT_REGTEST, JT_PROFILE,
               JT_LIST_LOAD, JT_LIST_TYPES, JT_LIST_TYPE_NAMES,
               JT_VIEW, JT_GEN, JT_GEN_TOKENS, JT_FORWARD
             };

struct setting {
  JobType job = JT_NOTHING;
  std::unique_ptr<UnicodeString> catalog;
  std::unique_ptr<UnicodeString> input_file;
  std::unique_ptr<UnicodeString> output_file;
  std::unique_ptr<UnicodeString> link_sources;
  std::unique_ptr<UnicodeString> type_desc;
  std::unique_ptr<UnicodeString> hrd_name;
  std::string log_file_prefix = "consoletools";
  std::string log_file_dir = "./";
  std::string log_level = "off";
  int profile_loops = 1;
  bool line_numbers = false;
  bool copyright = true;
  bool bom_output = true;
  bool html_esc = true;
  bool html_wrap = true;
} settings;

/** Reads and parse command line */
void readArgs(int argc, char* argv[])
{

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      settings.input_file = std::make_unique<UnicodeString>(argv[i]);
      continue;
    }

    if (argv[i][1] == 'p') {
      settings.job = JT_PROFILE;
      if (argv[i][2]) {
        settings.profile_loops = atoi(argv[i] + 2);
      }
      continue;
    }
    if (argv[i][1] == 'r') {
      settings.job = JT_REGTEST;
      continue;
    }
    if (argv[i][1] == 'f') {
      settings.job = JT_FORWARD;
      continue;
    }
    if (argv[i][1] == 'v') {
      settings.job = JT_VIEW;
      continue;
    }
    if (argv[i][1] == 'h' && argv[i][2] == 't') {
      settings.job = JT_GEN_TOKENS;
      continue;
    }
    if (argv[i][1] == 'h') {
      settings.job = JT_GEN;
      continue;
    }
    if (argv[i][1] == 'l' && argv[i][2] == 's' && (i + 1 < argc || argv[i][3])) {
      if (argv[i][3]) {
        settings.link_sources = std::make_unique<UnicodeString>(argv[i] + 3);
      } else {
        settings.link_sources = std::make_unique<UnicodeString>(argv[i + 1]);
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'l' && argv[i][2] == 'n') {
      settings.line_numbers = true;
      continue;
    }
    if (argv[i][1] == 'l' && argv[i][2] == 'l') {
      settings.job = JT_LIST_LOAD;
      continue;
    }
    if (argv[i][1] == 'l' && argv[i][2] == 't') {
      settings.job = JT_LIST_TYPE_NAMES;
      continue;
    }
    if (argv[i][1] == 'l') {
      settings.job = JT_LIST_TYPES;
      continue;
    }

    if (argv[i][1] == 'd' && argv[i][2] == 'c') {
      settings.copyright = false;
      continue;
    }
    if (argv[i][1] == 'd' && argv[i][2] == 'b') {
      settings.bom_output = false;
      continue;
    }
    if (argv[i][1] == 'd' && argv[i][2] == 's') {
      settings.html_esc = false;
      continue;
    }
    if (argv[i][1] == 'd' && argv[i][2] == 'h') {
      settings.html_wrap = false;
      continue;
    }

    if (argv[i][1] == 't' && (i + 1 < argc || argv[i][2])) {
      if (argv[i][2]) {
        settings.type_desc = std::make_unique<UnicodeString>(argv[i] + 2);
      } else {
        settings.type_desc = std::make_unique<UnicodeString>(argv[i + 1]);
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'o' && (i + 1 < argc || argv[i][2])) {
      if (argv[i][2]) {
        settings.output_file = std::make_unique<UnicodeString>(argv[i] + 2);
      } else {
        settings.output_file = std::make_unique<UnicodeString>(argv[i + 1]);
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'i' && (i + 1 < argc || argv[i][2])) {
      if (argv[i][2]) {
        settings.hrd_name = std::make_unique<UnicodeString>(argv[i] + 2);
      } else {
        settings.hrd_name = std::make_unique<UnicodeString>(argv[i + 1]);
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'c' && (i + 1 < argc || argv[i][2])) {
      if (argv[i][2]) {
        settings.catalog = std::make_unique<UnicodeString>(argv[i] + 2);
      } else {
        settings.catalog = std::make_unique<UnicodeString>(argv[i + 1]);
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'e' && argv[i][2] == 'h' && (i + 1 < argc || argv[i][3])) {
      if (argv[i][3]) {
        settings.log_file_prefix = std::string(argv[i] + 3);
      } else {
        settings.log_file_prefix = std::string(argv[i + 1]);
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'e' && argv[i][2] == 'd' && (i + 1 < argc || argv[i][3])) {
      if (argv[i][3]) {
        settings.log_file_dir = std::string(argv[i] + 3);
      } else {
        settings.log_file_dir = std::string(argv[i + 1]);
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'e' && argv[i][2] == 'l' && (i + 1 < argc || argv[i][3])) {
      if (argv[i][3]) {
        settings.log_level = std::string(argv[i] + 3);
      } else {
        settings.log_level = std::string(argv[i + 1]);
        i++;
      }
      continue;
    }
    if (argv[i][1]) {
      fprintf(stderr, "WARNING: unknown option '-%s'\n", argv[i] + 1);
    }
  }
}

/** Prints usage. */
void printError()
{
  fprintf(stderr,
    "Usage: colorer (command) (parameters) (logging parameters) [<filename>]\n"
          " Commands:\n"
          "  -l         Lists all available languages\n"
          "  -lt        Lists all available languages (HRC types)\n"
          "  -ll        Lists and loads full HRC database\n"
          "  -r         RE tests\n"
          "  -h         Generates plain coloring from <filename> (uses 'rgb' hrd class)\n"
          "  -ht        Generates plain coloring from <filename> using tokens output\n"
          "  -v         Runs viewer on file <fname> (uses 'console' hrd class)\n"
          "  -p<n>      Runs parser in profile mode (if <n> specified, makes <n> loops)\n"
          "  -f         Forwards input file into output with specified encodings\n"
          " Parameters:\n"
          "  -c<path>   Uses specified 'catalog.xml' file\n"
          "  -i<name>   Loads specified hrd rules from catalog\n"
          "  -t<type>   Tries to use type <type> instead of type autodetection\n"
          "  -ls<name>  Use file <name> as input linking data source for href generation\n"
          "  -o<name>   Use file <name> as output stream\n"
          "  -ln        Add line numbers into the colorized file\n"
          "  -db        Disable BOM(ZWNBSP) start symbol output in Unicode encodings\n"
          "  -dc        Disable information header in generator's output\n"
          "  -ds        Disable HTML symbol substitutions in generator's output\n"
          "  -dh        Disable HTML header and footer output\n"
          " Logging parameters (default logging off):\n"
          "  -eh<name>  Log file name prefix\n"
          "  -ed<name>  Log file directory\n"
          "  -el<name>  Log level (off, debug, info, warning, error)\n"
         );
}

void initConsoleTools(ConsoleTools &ct)
{
  if (settings.input_file) {
    ct.setInputFileName(*settings.input_file);
  }
  if (settings.catalog) {
    ct.setCatalogPath(*settings.catalog);
  }
  if (settings.link_sources) {
    ct.setLinkSource(*settings.link_sources);
  }
  if (settings.output_file) {
    ct.setOutputFileName(*settings.output_file);
  }
  if (settings.type_desc) {
    ct.setTypeDescription(*settings.type_desc);
  }
  if (settings.hrd_name) {
    ct.setHRDName(*settings.hrd_name);
  }
  ct.addLineNumbers(settings.line_numbers);
  ct.setCopyrightHeader(settings.copyright);
  ct.setHtmlEscaping(settings.html_esc);
  ct.setHtmlWrapping(settings.html_wrap);
  ct.setBomOutput(settings.bom_output);
}

int workIt()
{

  ConsoleTools ct;
  initConsoleTools(ct);

  if (settings.copyright) {
    fprintf(stdout, "\nColorer console tools, version %s\n", COLORER_VERSION);
    fprintf(stdout, "%s \n\n", COLORER_COPYRIGHT);
  }

  try {
    switch (settings.job) {
      case JT_REGTEST:
        ConsoleTools::RETest();
        break;
      case JT_PROFILE:
        ct.profile(settings.profile_loops);
        break;
      case JT_LIST_LOAD:
        ct.listTypes(true, false);
        break;
      case JT_LIST_TYPES:
        ct.listTypes(false, false);
        break;
      case JT_LIST_TYPE_NAMES:
        ct.listTypes(false, true);
        break;
      case JT_VIEW:
        ct.viewFile();
        break;
      case JT_GEN:
        ct.genOutput();
        break;
      case JT_GEN_TOKENS:
        ct.genTokenOutput();
        break;
      case JT_FORWARD:
        ct.forward();
        break;
      default:
        printError();
        break;
    }
  } catch (Exception &e) {
    spdlog::error("{0}", e.what());
    fprintf(stderr, "%s", e.what());
    return -1;
  }
  return 0;
}

/** Creates ConsoleTools class instance and runs it.
*/
int main(int argc, char* argv[])
{
  readArgs(argc, argv);

  auto level = spdlog::level::from_str(settings.log_level);
  if (level!= spdlog::level::off) {
    try {
      std::string file_name = settings.log_file_dir + "/" + settings.log_file_prefix + ".log";
      auto logger = spdlog::basic_logger_mt("main", file_name);
      spdlog::set_default_logger(logger);
      logger->set_level(level);
    } catch (std::exception &e){
      fprintf(stderr, "%s\n", e.what());
      return -1;
    }
  }

  return workIt();
}
