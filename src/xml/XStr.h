#ifndef _COLORER2_XSTR_H_
#define _COLORER2_XSTR_H_

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
  XStr(const std::string &toTranscode);
  XStr(const std::string* toTranscode);
  ~XStr();

  const char* get_char() const;
  const std::string get_stdstr() const;
  const XMLCh* get_xmlchar();

private:
  XStr(XStr const &);
  XStr &operator=(XStr const &);
  XStr(XStr &&);
  XStr &operator=(XStr &&);

  friend std::ostream &operator<<(std::ostream &stream, const XStr &x);

  XMLCh* _xmlch;
  std::string _string;
};

#endif //_COLORER2_XSTR_H_
