#ifndef _COLORER2_LOGFILESINK_H_
#define _COLORER2_LOGFILESINK_H_

#include <string>
#include <memory>
#include <g3log/logmessage.hpp>
#include <g3log/std2_make_unique.hpp>

using namespace g3;

static const std::string default_format = "%Y/%m/%d %H:%M:%S";
static const std::string long_format = "%Y/%m/%d %H:%M:%S.%f6";

class LogFileSink
{
public:
  LogFileSink(const std::string &log_prefix, const std::string &log_directory, 
               const bool show_microseconds = false, const bool standing_name = false);
  virtual ~LogFileSink();

  void fileWrite(const LogMessageMover message);

private:
  std::string _log_file_with_path;
  std::string _log_prefix_backup; // needed in case of future log file changes of directory
  std::unique_ptr <std::ofstream> _outptr;
  bool show_microseconds;
  bool standing_name;

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
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Colorer Library.
 *
 * The Initial Developer of the Original Code is
 * Cail Lomecb <irusskih at gmail dot com>.
 * Portions created by the Initial Developer are Copyright (C) 1999-2009
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s): see file CONTRIBUTORS
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
