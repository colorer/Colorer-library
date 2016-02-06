#include <iostream>
#include <fstream>
#include <algorithm>
#include <g3log/g3log.hpp>
#include <utils/LogFileSink.h>

static const std::string file_name_time_formatted = "%Y%m%d-%H%M%S";

LogFileSink::LogFileSink(const std::string &log_prefix, const std::string &log_directory, const bool show_microseconds_,
                          const bool standing_name_)
  : _log_file_with_path(log_directory), _log_prefix_backup(log_prefix), _outptr(new std::ofstream), 
    show_microseconds(show_microseconds_), standing_name(standing_name_)
{
  _log_prefix_backup = prefixSanityFix(log_prefix);
  if (!isValidFilename(_log_prefix_backup)) {
    std::cerr << "LogFileSink: forced abort due to illegal log prefix [" << log_prefix << "]" << std::endl;
    abort();
  }

  std::string file_name;
  if (standing_name_) {
    file_name = _log_prefix_backup + ".log";
  } else {
    file_name = createLogFileName(_log_prefix_backup);
  }
  _log_file_with_path = pathSanityFix(_log_file_with_path, file_name);
  _outptr = createLogFile(_log_file_with_path);

  if (!_outptr) {
    std::cerr << "Cannot write log file to location, attempting current directory" << std::endl;
    _log_file_with_path = "./" + file_name;
    _outptr = createLogFile(_log_file_with_path);
  }

  LogMessage uniqueMsg(__FILE__, __LINE__, __PRETTY_FUNCTION__, INFO);
  uniqueMsg.write() = "start logging";
  fileWrite(LogMessageMover(std::move(uniqueMsg)));
}

LogFileSink::~LogFileSink()
{
  LogMessage uniqueMsg(__FILE__, __LINE__, __PRETTY_FUNCTION__, INFO);
  uniqueMsg.write() = "end logging";
  fileWrite(LogMessageMover(std::move(uniqueMsg)));
}

std::string LogFileSink::formatMessage(LogMessageMover message)
{
  auto level_value = message.get()._level.value;
  const char* levelstr = (
                           level_value == DEBUG.value ? "DEBUG  "
                           : level_value == ERROR.value ? "ERROR  "
                           : level_value == INFO.value ? "INFO   "
                           : level_value == WARNING.value ? "WARNING"
                           : level_value == ERRORF.value ? "ERRORF "
                           : level_value == FATAL.value ? "FATAL  "
                           : ""
                         );
  std::ostringstream oss;

  oss << "[" << message.get().timestamp();
  if (show_microseconds) {
    oss << "." << message.get().microseconds();
  }
  oss << "]";
  oss << " [" << levelstr << "]" << '\t';

  // Non-fatal Log Message
  if (!message.get().wasFatal()) {
    oss << message.get().message() << "\n";
    return oss.str();
  }

  if (internal::FATAL_SIGNAL.value == message.get()._level.value) {
    oss.str(""); // clear any previous text and formatting
    oss << "\n" << message.get().timestamp();
    if (show_microseconds) {
      oss << "." << message.get().microseconds();
    }
    oss << "\n\n***** FATAL SIGNAL RECEIVED ******* " << std::endl;
    oss << '"' << message.get().message() << '"';
    return oss.str();
  }

  // Not crash scenario but LOG or CONTRACT
  if (FATAL.value == level_value) {
    oss << "\n\t*******\tEXIT trigger caused by LOG(FATAL) entry: \n\t";
    oss << '"' << message.get().message() << '"';
  } else if (internal::CONTRACT.value == level_value) {
    oss << "\n\t  *******\tEXIT trigger caused by broken Contract: CHECK(" << message.get().expression() << ")\n\t";
    oss << '"' << message.get().message() << '"';
  } else {
    oss << "\n\t*******\tUNKNOWN Log Message Type\n" << '"' << message.get().message() << '"';
  }
  return oss.str();
}

void LogFileSink::fileWrite(const LogMessageMover message)
{
  std::ofstream &out(filestream());
  out << formatMessage(message) << std::flush;
}

// check for filename validity -  filename should not be part of PATH
bool LogFileSink::isValidFilename(const std::string &prefix_filename)
{
  std::string illegal_characters("/,|<>:#$%{}()[]\'\"^!?+* ");
  size_t pos = prefix_filename.find_first_of(illegal_characters, 0);
  if (pos != std::string::npos) {
    std::cerr << "Illegal character [" << prefix_filename.at(pos) << "] in logname prefix: " << "[" <<
              prefix_filename << "]" << std::endl;
    return false;
  } else if (prefix_filename.empty()) {
    std::cerr << "Empty filename prefix is not allowed" << std::endl;
    return false;
  }

  return true;
}

std::string LogFileSink::prefixSanityFix(std::string prefix)
{
  prefix.erase(std::remove_if(prefix.begin(), prefix.end(), ::isspace), prefix.end());
  prefix.erase(std::remove(prefix.begin(), prefix.end(), '/'), prefix.end());
  prefix.erase(std::remove(prefix.begin(), prefix.end(), '\\'), prefix.end());
  prefix.erase(std::remove(prefix.begin(), prefix.end(), '.'), prefix.end());
  prefix.erase(std::remove(prefix.begin(), prefix.end(), ':'), prefix.end());
  if (!isValidFilename(prefix)) {
    return {};
  }
  return prefix;
}

std::string LogFileSink::pathSanityFix(std::string path, const std::string &file_name)
{
  // Unify the delimeters,. maybe sketchy solution but it seems to work
  // on at least win7 + ubuntu. All bets are off for older windows
  std::replace(path.begin(), path.end(), '\\', '/');

  // clean up in case of multiples
  auto contains_end = [&](std::string & in) -> bool {
    size_t size = in.size();
    if (!size)
    {
      return false;
    }
    char end = in[size - 1];
    return (end == '/' || end == ' ');
  };

  while (contains_end(path)) {
    path.erase(path.size() - 1);
  }

  if (!path.empty()) {
    path.insert(path.end(), '/');
  }

  path.insert(path.size(), file_name);
  return path;
}

std::string LogFileSink::createLogFileName(const std::string &verified_prefix)
{
  std::stringstream oss_name;
  oss_name << verified_prefix << ".";
  oss_name << localtime_formatted(systemtime_now(), file_name_time_formatted);
  oss_name << ".log";
  return oss_name.str();
}

bool LogFileSink::openLogFile(const std::string &complete_file_with_path, std::ofstream &outstream)
{
  std::ios_base::openmode mode = std::ios_base::out; // for clarity: it's really overkill since it's an ofstream
  if (standing_name){
    mode |= std::ios_base::app;
  } else {
    mode |= std::ios_base::trunc;
  }
  outstream.open(complete_file_with_path, mode);
  if (!outstream.is_open()) {
    std::ostringstream ss_error;
    ss_error << "FILE ERROR: could not open log file:[" << complete_file_with_path << "]";
    ss_error << "\n\t\t std::ios_base state = " << outstream.rdstate();
    std::cerr << ss_error.str().c_str() << std::endl;
    outstream.close();
    return false;
  }
  return true;
}

std::unique_ptr<std::ofstream> LogFileSink::createLogFile(const std::string &file_with_full_path)
{
  std::unique_ptr<std::ofstream> out(new std::ofstream);
  std::ofstream &stream(*(out.get()));
  bool success_with_open_file = openLogFile(file_with_full_path, stream);
  if (!success_with_open_file) {
    out.release();
  }
  return out;
}


