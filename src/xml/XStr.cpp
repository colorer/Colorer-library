#include <xml/XStr.h>


XStr::XStr(const XMLCh* const toTranscode)
{
  _xmlch = xercesc::XMLString::replicate(toTranscode);
  _string = xercesc::XMLString::transcode(toTranscode);
}

XStr::XStr(const std::string &toTranscode)
{
  if (toTranscode.empty()) {
    _xmlch = nullptr;
  } else {
    _string = toTranscode;
    _xmlch = xercesc::XMLString::transcode(toTranscode.c_str());
  }
}

XStr::XStr(const std::string* toTranscode)
{
  if (toTranscode == nullptr || toTranscode->empty()) {
    _xmlch = nullptr;
  } else {
    _string = *toTranscode;
    _xmlch = xercesc::XMLString::transcode(toTranscode->c_str());
  }
}

XStr::~XStr()
{
  xercesc::XMLString::release(&_xmlch);
}

std::ostream &operator<<(std::ostream &stream, const XStr &x)
{
  if (x._string.empty()) {
    stream << "\\0";
  } else {
    stream << x._string;
  }
  return stream;
}

const char* XStr::get_char() const
{
  if (_string.empty()) {
    return nullptr;
  } else {
    return _string.c_str();
  }
}

const std::string XStr::get_stdstr() const
{
  return _string;
}

const XMLCh* XStr::get_xmlchar()
{
  return _xmlch;
}
