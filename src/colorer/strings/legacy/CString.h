#ifndef _COLORER_CSTRING_H_
#define _COLORER_CSTRING_H_

#include <colorer/strings/legacy/String.h>
#include <colorer/strings/legacy/UnicodeString.h>

/** Dynamic string class.
    Simple unicode wrapper over any other source.
    @ingroup unicode
*/
class CString : public String
{
public:
  /** String clone operator */
  CString &operator=(const CString &cstring);

  /** Creates string from byte stream with encoding autodetecting
      @param stream Input raw byte stream, can't be null.
      @param size Size of input buffer
      @param def_encoding Default encoding to be used, if no other
             variants found.
  */
  CString(const byte* stream, int32_t size, int def_encoding = -1);

  /** String from single-byte character buffer.
      @param string Character buffer, can't be null.
      @param s Start string position. Zero - create from start of buffer.
      @param l Length of created string. If npos, autodetects string length with
             last zero byte.
      @param encoding Encoding, to use for char2unicode transformations.
             If -1, default encoding will be used.
  */
  CString(const char* string, int32_t s = 0, int32_t l = npos, int encoding = -1);

  /** String from unicode two-byte character buffer.
      @param string Unicode character buffer, can't be null.
      @param s Start string position. Zero - create from start of buffer.
      @param l Length of created string. If npos, autodetects string length with
             last zero char.
  */
  CString(const w2char* string, int32_t s = 0, int32_t l = npos);
  /** String from UCS4 four-byte character buffer.
      @param string UCS4 unicode character buffer, can't be null.
      @param s Starting string position. Zero - create from start of buffer.
      @param l Length of created string. If npos, autodetects string length with
             last zero char.
  */
  CString(const w4char* string, int32_t s = 0, int32_t l = npos);

#if (__WCHAR_MAX__ > 0xffff)
  CString(const wchar* string, int32_t s = 0, int32_t l = npos);
#endif

  /** String from any @c String implementing interface.
      @param cstring String class instance, can't be null.
      @param s Starting string position.
      @param l Length of created string. If npos, autodetects string length with
             cstring.length() call.
  */
  CString(const String* cstring, int32_t s = 0, int32_t l = npos);

  /** String from any @c String implementing interface.
      @param cstring String class instance.
      @param s Starting string position.
      @param l Length of created string. If npos, autodetects string length with
             cstring.length() call.
  */
  CString(const String &cstring, int32_t s = 0, int32_t l = npos);

  /** Empty String */
  CString();
  ~CString();

  CString(CString const &) = delete;
  CString(CString &&cstring);
  CString &operator=(CString &&cstring);

  wchar operator[](int32_t i) const override;
  int32_t length() const override;

protected:
  enum EStreamType {
    ST_CHAR = 0,
    ST_UTF16,
    ST_UTF16_BE,
    ST_CSTRING,
    ST_UTF8,
    ST_UTF32,
    ST_UTF32_BE
  };

  EStreamType type;
  int encodingIdx;
  union {
    const char* str;
    const w2char* w2str;
    const w4char* w4str;
    const String* cstr;
    wchar* stream_wstr;
  };
  int32_t start;
  int32_t len;

};

inline CString::CString(CString &&cstring):
  type(cstring.type),
  encodingIdx(cstring.encodingIdx),
  cstr(cstring.cstr),
  start(cstring.start),
  len(cstring.len)
{

  cstring.cstr = nullptr;
  cstring.len = 0;
  cstring.start = 0;
  cstring.type = ST_CHAR;
  cstring.encodingIdx = -1;
}

inline CString &CString::operator=(CString &&cstring)
{
  if (this != &cstring) {
    type = cstring.type;
    len = cstring.len;
    start = cstring.start;
    cstr = cstring.cstr;
    encodingIdx = cstring.encodingIdx;

    cstring.cstr = nullptr;
    cstring.len = 0;
    cstring.start = 0;
    cstring.type = ST_CHAR;
    cstring.encodingIdx = -1;
  }
  return *this;
}

#endif



