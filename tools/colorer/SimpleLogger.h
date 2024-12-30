#ifndef SIMPLELOGGER_H
#define SIMPLELOGGER_H

#include <fstream>
#include <locale>
#include "colorer/common/Logger.h"

class SimpleLogger : public Logger
{
 public:
  static const std::string_view LogLevelStr[];

  SimpleLogger(const std::string_view& filename, const std::string& log_level);

  SimpleLogger(const std::string_view& filename, Logger::LogLevel log_level);

  ~SimpleLogger() override;

  void open_logfile();

  void log(Logger::LogLevel level, const char* /*filename_in*/, int /*line_in*/, const char* /*funcname_in*/,
           const char* message) override;

  static Logger::LogLevel getLogLevel(const std::string& log_level);

  void setLogLevel(Logger::LogLevel level);
  void flush() override;

 private:
  std::ofstream ofs;
  Logger::LogLevel current_level;
  std::string log_filename;
};

#endif  // SIMPLELOGGER_H
