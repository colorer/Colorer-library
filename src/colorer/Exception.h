#ifndef _COLORER_EXCEPTION_H__
#define _COLORER_EXCEPTION_H__

#include "colorer/Common.h"
#include <exception>

/** Exception class.
    Defines throwable exception.
    @ingroup common
*/
class Exception : public std::exception
{
 public:
  explicit Exception(const char* msg) noexcept;
  /** Creates exception with string message
   */
  explicit Exception(const UnicodeString& msg) noexcept;

  /** Default destructor
   */
  ~Exception() noexcept override = default;

  Exception& operator=(const Exception& e) = delete;

  /** Returns exception message
   */
  [[nodiscard]] const char* what() const noexcept override;

 protected:
  /** Internal message container
   */
  std::string what_str;
};

/**
    InputSourceException is thrown, if some IO error occurs.
    @ingroup common
*/
class InputSourceException : public Exception
{
 public:
  explicit InputSourceException(const UnicodeString& msg) noexcept;
};

#endif
