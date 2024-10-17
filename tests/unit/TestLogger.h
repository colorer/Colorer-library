#ifndef TESTLOGGER_H
#define TESTLOGGER_H

#include <iostream>
#include "colorer/common/Logger.h"

class TestLogger : public Logger
{
 public:
  static constexpr std::string_view LogLevelStr[] {"off", "error", "warning", "info", "debug", "trace"};

  TestLogger() { current_level = LogLevel::LOG_WARN; }

  ~TestLogger() override = default;

  void log(Logger::LogLevel level, const char* /*filename_in*/, int /*line_in*/, const char* /*funcname_in*/,
           const char* message) override
  {
    if (level > current_level) {
      return;
    }
    log_messages.push_back(message);
    //std::cerr << message << '\n';
  }

  bool message_exists() const { return !log_messages.empty(); }
  bool message_print(bool waiting_exist = false) const
  {
    if (!waiting_exist && message_exists()) {
      print_messages();
    }
    return !log_messages.empty();
  }
  void clean_messages() { log_messages.clear(); }
  void print_messages() const
  {
    for (const auto& msg : log_messages) {
      std::cerr << msg << '\n';
    }
  }

  std::vector<std::string> log_messages;

 private:
  Logger::LogLevel current_level = Logger::LOG_OFF;
};

#endif  // TESTLOGGER_H
