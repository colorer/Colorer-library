#ifndef _COLORER_XSTR_H_
#define _COLORER_XSTR_H_

#include <colorer/common/UnicodeString.h>
#include <memory>
#include <ostream>
#include <string>
#include <xercesc/util/XMLString.hpp>

/**
 * This is a simple class that lets us do easy trancoding of
 * XMLCh data to char* data.
 *
 */
class XStr
{
public:
  XStr(const XMLCh* const toTranscode);
  explicit XStr(const std::string &toTranscode);
  explicit XStr(const std::string* toTranscode);
  ~XStr();

  [[nodiscard]] const char* get_char() const;
  [[nodiscard]] const std::string* get_stdstr() const;
  [[nodiscard]] const XMLCh* get_xmlchar() const;

  XStr(XStr const &) = delete;
  XStr &operator=(XStr const &) = delete;
  XStr(XStr &&) = delete;
  XStr &operator=(XStr &&) = delete;
private:

  friend std::ostream &operator<<(std::ostream &stream, const XStr &x);

  std::unique_ptr<XMLCh> _xmlch;
  std::string _string;
};

#endif //_COLORER_XSTR_H_

