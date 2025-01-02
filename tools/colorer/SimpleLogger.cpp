#include "SimpleLogger.h"

const std::string_view SimpleLogger::LogLevelStr[] {"off", "error", "warning", "info", "debug", "trace"};

SimpleLogger::SimpleLogger(const std::string_view& filename, const std::string& log_level)
{
  current_level = getLogLevel(log_level);
  log_filename = filename;
  open_logfile();
}

SimpleLogger::SimpleLogger(const std::string_view& filename, const Logger::LogLevel log_level)
{
  current_level = log_level;
  log_filename = filename;
  open_logfile();
}

SimpleLogger::~SimpleLogger()
{
  ofs.close();
}

void SimpleLogger::open_logfile()
{
  if (current_level == LOG_OFF || ofs.is_open()) {
    return;
  }
  ofs.open(log_filename.c_str(), std::ofstream::out | std::ofstream::app);
  if (!ofs.is_open()) {
    throw std::runtime_error("Could not open file to write logs: " + log_filename);
  }
}

void SimpleLogger::log(Logger::LogLevel level, const char* /*filename_in*/, int /*line_in*/,
                       const char* /*funcname_in*/, const char* message)
{
  if (level > current_level) {
    return;
  }
  std::time_t const t = std::time(nullptr);
  char mbstr[30];
  std::strftime(mbstr, sizeof(mbstr), "%FT%T", std::localtime(&t));
  ofs << "[" << mbstr << "] "
      << "[" << LogLevelStr[level] << "] ";
  ofs << message << '\n';
}

Logger::LogLevel SimpleLogger::getLogLevel(const std::string& log_level)
{
  int i = 0;
  for (auto it : LogLevelStr) {
    if (log_level == it) {
      return static_cast<Logger::LogLevel>(i);
    }
    i++;
  }
  if (log_level == "warn") {
    return Logger::LOG_WARN;
  }
  return Logger::LOG_OFF;
}

void SimpleLogger::setLogLevel(Logger::LogLevel level)
{
  ofs.flush();
  if (current_level == Logger::LOG_OFF) {
    current_level = level;
    open_logfile();
  }else {
    current_level = level;
  }
}

void SimpleLogger::flush()
{
  ofs.flush();
}