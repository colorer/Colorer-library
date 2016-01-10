#ifndef _COLORER2_LOGFILESINK_H_
#define _COLORER2_LOGFILESINK_H_

#include <string>
#include <memory>
#include <g3log/logmessage.hpp>
#include <g3log/std2_make_unique.hpp>

using namespace g3;

class LogFileSink
{
public:
  LogFileSink(const std::string &log_prefix, const std::string &log_directory, const bool show_microseconds);
  virtual ~LogFileSink();

  void fileWrite(const LogMessageMover message);

private:
  std::string _log_file_with_path;
  std::string _log_prefix_backup; // needed in case of future log file changes of directory
  std::unique_ptr <std::ofstream> _outptr;
  bool show_microseconds;

  std::ofstream &filestream()
  {
    return *(_outptr.get());
  }

  std::string prefixSanityFix(std::string prefix);
  std::string pathSanityFix(std::string path, const std::string &file_name);
  bool isValidFilename(const std::string &prefix_filename);
  std::string createLogFileName(const std::string &verified_prefix);
  bool openLogFile(const std::string &complete_file_with_path, std::ofstream &outstream);
  std::unique_ptr <std::ofstream> createLogFile(const std::string &file_with_full_path);
  std::string formatMessage(LogMessageMover message);

  LogFileSink &operator=(const LogFileSink &) = delete;
  LogFileSink(const LogFileSink &other) = delete;

};

#endif // _COLORER2_LOGFILESINK_H_
