#include <colorer/version.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include "ConsoleTools.h"
#include "SimpleLogger.h"
#include "colorer/utils/Environment.h"

/** Internal run action type */
enum class JobType {
  JT_NOTHING,
  JT_REGTEST,
  JT_PROFILE,
  JT_LIST_LOAD,
  JT_LIST_TYPES,
  JT_LIST_TYPE_NAMES,
  JT_LOAD_TYPE,
  JT_VIEW,
  JT_GEN,
  JT_GEN_TOKENS
};

struct setting
{
  JobType job = JobType::JT_NOTHING;
  std::unique_ptr<UnicodeString> catalog;
  std::unique_ptr<UnicodeString> hrcsettings;
  std::unique_ptr<UnicodeString> user_hrc_path;
  std::unique_ptr<UnicodeString> user_hrd_path;
  std::unique_ptr<UnicodeString> input_file;
  std::unique_ptr<UnicodeString> output_file;
  std::unique_ptr<UnicodeString> link_sources;
  std::unique_ptr<UnicodeString> type_desc;
  std::unique_ptr<UnicodeString> hrd_name;
  std::string log_file_prefix = "consoletools";
  std::string log_file_dir = ".";
  std::string log_level = "off";
  int profile_loops = 1;
  bool line_numbers = false;
  bool copyright = true;
  bool bom_output = true;
  bool html_esc = true;
  bool html_wrap = true;
} settings;

std::unique_ptr<SimpleLogger> logger;

/** Reads and parse command line */
void readArgs(int argc, char* argv[])
{
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      settings.input_file = std::make_unique<UnicodeString>(argv[i]);
      continue;
    }

    if (argv[i][1] == 'p') {
      settings.job = JobType::JT_PROFILE;
      if (argv[i][2]) {
        settings.profile_loops = atoi(argv[i] + 2);
      }
      continue;
    }
    if (argv[i][1] == 'r') {
      settings.job = JobType::JT_REGTEST;
      continue;
    }
    if (argv[i][1] == 'v') {
      settings.job = JobType::JT_VIEW;
      continue;
    }
    if (argv[i][1] == 'h' && argv[i][2] == 't') {
      settings.job = JobType::JT_GEN_TOKENS;
      continue;
    }
    if (argv[i][1] == 'h') {
      settings.job = JobType::JT_GEN;
      continue;
    }
    if (argv[i][1] == 'l' && argv[i][2] == 's' && (i + 1 < argc || argv[i][3])) {
      if (argv[i][3]) {
        settings.link_sources = std::make_unique<UnicodeString>(argv[i] + 3);
      }
      else {
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
      settings.job = JobType::JT_LIST_LOAD;
      continue;
    }
    if (argv[i][1] == 'l' && argv[i][2] == 't') {
      settings.job = JobType::JT_LIST_TYPE_NAMES;
      continue;
    }
    if (argv[i][1] == 'l' && argv[i][2] == 'd') {
      settings.job = JobType::JT_LOAD_TYPE;
      continue;
    }
    if (argv[i][1] == 'l') {
      settings.job = JobType::JT_LIST_TYPES;
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
      }
      else {
        settings.type_desc = std::make_unique<UnicodeString>(argv[i + 1]);
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'o' && (i + 1 < argc || argv[i][2])) {
      if (argv[i][2]) {
        settings.output_file = std::make_unique<UnicodeString>(argv[i] + 2);
      }
      else {
        settings.output_file = std::make_unique<UnicodeString>(argv[i + 1]);
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'i' && (i + 1 < argc || argv[i][2])) {
      if (argv[i][2]) {
        settings.hrd_name = std::make_unique<UnicodeString>(argv[i] + 2);
      }
      else {
        settings.hrd_name = std::make_unique<UnicodeString>(argv[i + 1]);
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'c' && argv[i][2] == 's' && (i + 1 < argc || argv[i][3])) {
      if (argv[i][3]) {
        settings.hrcsettings = std::make_unique<UnicodeString>(argv[i] + 3);
      }
      else {
        settings.hrcsettings = std::make_unique<UnicodeString>(argv[i + 1]);
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'c' && argv[i][2] == 'u' && (i + 1 < argc || argv[i][3])) {
      if (argv[i][3]) {
        settings.user_hrc_path = std::make_unique<UnicodeString>(argv[i] + 3);
      }
      else {
        settings.user_hrc_path = std::make_unique<UnicodeString>(argv[i + 1]);
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'c' && argv[i][2] == 'd' && (i + 1 < argc || argv[i][3])) {
      if (argv[i][3]) {
        settings.user_hrd_path = std::make_unique<UnicodeString>(argv[i] + 3);
      }
      else {
        settings.user_hrd_path = std::make_unique<UnicodeString>(argv[i + 1]);
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'c' && (i + 1 < argc || argv[i][2])) {
      if (argv[i][2]) {
        settings.catalog = std::make_unique<UnicodeString>(argv[i] + 2);
      }
      else {
        settings.catalog = std::make_unique<UnicodeString>(argv[i + 1]);
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'e' && argv[i][2] == 'h' && (i + 1 < argc || argv[i][3])) {
      if (argv[i][3]) {
        settings.log_file_prefix = std::string(argv[i] + 3);
      }
      else {
        settings.log_file_prefix = std::string(argv[i + 1]);
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'e' && argv[i][2] == 'd' && (i + 1 < argc || argv[i][3])) {
      if (argv[i][3]) {
        settings.log_file_dir = std::string(argv[i] + 3);
      }
      else {
        settings.log_file_dir = std::string(argv[i + 1]);
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'e' && argv[i][2] == 'l' && (i + 1 < argc || argv[i][3])) {
      if (argv[i][3]) {
        settings.log_level = std::string(argv[i] + 3);
      }
      else {
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
void printUsage()
{
  fprintf(stdout,
          "Usage: colorer COMMAND [PARAMETERS] [LOGGING PARAMETERS] [FILE]\n"
          " Commands:\n"
          "  -l         Lists all available languages\n"
          "  -lt        Lists all available languages (HRC types)\n"
          "  -ll        Lists and loads full HRC database\n"
          "  -ld        Load type from HRC database (-t<type>)\n"
          "  -r         RE tests\n"
          "  -h         Generates plain coloring from <filename> (uses 'rgb' hrd class)\n"
          "  -ht        Generates plain coloring from <filename> using tokens output\n"
          "  -v         Runs viewer on file <fname> (uses 'console' hrd class)\n"
          "  -p<n>      Runs parser in profile mode (if <n> specified, makes <n> loops)\n"
          " Parameters:\n"
          "  -c<path>   Uses specified 'catalog.xml' file\n"
          "  -cs<path>  Uses specified 'hrcsettings.xml' file\n"
          "  -cu<path>  Load user hrc-files from path\n"
          "  -cd<path>  Load user hrd-files from path\n"
          "  -i<name>   Loads specified hrd rules from catalog\n"
          "  -t<type>   Tries to use type <type> instead of type autodetection\n"
          "  -ls<name>  Use file <name> as input linking data source for href generation\n"
          "  -o<name>   Use file <name> as output stream\n"
          "  -ln        Add line numbers into the colorized file\n"
          "  -db        Disable BOM start symbol output in Unicode encodings\n"
          "  -dc        Disable information header in generator's output\n"
          "  -ds        Disable HTML symbol substitutions in generator's output\n"
          "  -dh        Disable HTML header and footer output\n"
          " Logging parameters (default logging off):\n"
          "  -eh<name>  Log file name prefix\n"
          "  -ed<name>  Log file directory\n"
          "  -el<name>  Log level (off, debug, info, warning, error). Default value is off.\n");
  fprintf(stdout, "\ncolorer console tools, version %s\nhome page: <https://github.com/colorer/Colorer-library>",
          COLORER_VERSION);
}

void initConsoleTools(ConsoleTools& ct)
{
  auto cur_dir = colorer::Environment::getCurrentDir();

  if (settings.input_file) {
    auto full_path = UStr::to_unistr(colorer::Environment::getClearFilePath(&cur_dir, settings.input_file.get()));
    ct.setInputFileName(full_path);
  }
  if (settings.catalog) {
    auto full_path = UStr::to_unistr(colorer::Environment::getClearFilePath(&cur_dir, settings.catalog.get()));
    ct.setCatalogPath(full_path);
  }
  if (settings.hrcsettings) {
    auto full_path = UStr::to_unistr(colorer::Environment::getClearFilePath(&cur_dir, settings.hrcsettings.get()));
    ct.setHrcSettingsPath(full_path);
  }
  if (settings.user_hrc_path) {
    auto full_path = UStr::to_unistr(colorer::Environment::getClearFilePath(&cur_dir, settings.user_hrc_path.get()));
    ct.setUserHrcPath(full_path);
  }
  if (settings.user_hrd_path) {
    auto full_path = UStr::to_unistr(colorer::Environment::getClearFilePath(&cur_dir, settings.user_hrd_path.get()));
    ct.setUserHrdPath(full_path);
  }
  if (settings.link_sources) {
    auto full_path = UStr::to_unistr(colorer::Environment::getClearFilePath(&cur_dir, settings.link_sources.get()));
    ct.setLinkSource(full_path);
  }
  if (settings.output_file) {
    auto full_path = UStr::to_unistr(colorer::Environment::getClearFilePath(&cur_dir, settings.output_file.get()));
    ct.setOutputFileName(full_path);
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

  try {
    switch (settings.job) {
      case JobType::JT_REGTEST:
        ConsoleTools::RETest();
        break;
      case JobType::JT_PROFILE:
        ct.profile(settings.profile_loops);
        break;
      case JobType::JT_LIST_LOAD:
        ct.listTypes(true, false);
        break;
      case JobType::JT_LIST_TYPES:
        ct.listTypes(false, false);
        break;
      case JobType::JT_LIST_TYPE_NAMES:
        ct.listTypes(false, true);
        break;
      case JobType::JT_LOAD_TYPE:
        ct.loadType();
        break;
      case JobType::JT_VIEW:
        ct.viewFile();
        break;
      case JobType::JT_GEN:
        ct.genOutput();
        break;
      case JobType::JT_GEN_TOKENS:
        ct.genTokenOutput();
        break;
      default:
        printUsage();
        break;
    }
  } catch (Exception& e) {
    COLORER_LOG_ERROR(e.what());
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

  auto u_loglevel = UnicodeString(settings.log_level.c_str());
  auto level = SimpleLogger::getLogLevel(UStr::to_stdstr(&u_loglevel));
  if (level != Logger::LOG_OFF) {
    std::string const file_name = settings.log_file_dir + "/" + settings.log_file_prefix + ".log";

    logger = std::make_unique<SimpleLogger>(file_name, level);
    Log::registerLogger(*logger);
  }

  return workIt();
}
