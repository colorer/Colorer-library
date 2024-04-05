#ifndef _COLORER_SSTRING_H_
#define _COLORER_SSTRING_H_

#include <colorer/strings/legacy/String.h>
#include <memory>
class DString;

/**
 * Unicode string.
 * @ingroup unicode
*/
class SString : public String
{
public:
  /**
  * Empty string constructor
  */
  SString();
  /**
   * String constructor from String source
   * @param cstring source string, can't be null.
   */
  SString(const String* cstring, int32_t s = 0, int32_t l = npos);

  /**
   * String constructor from String source
   * @param cstring source string, can't be null.
   */
  SString(const String &cstring, int32_t s = 0, int32_t l = npos);
  SString(const SString &cstring);

  /**
   * String constructor from char stream
   * @param str source string, can't be null.
   */
  SString(const char* string, int32_t s = 0, int32_t l = npos);
#ifndef WIN32
  SString(const w2char* string, int32_t s = 0, int32_t l = npos);
#endif
  SString(const w4char* string, int32_t s = 0, int32_t l = npos);
  SString(char* str, int enc = -1);
  SString(const wchar_t* str);

  /**
   * String constructor from integer number
   */
  SString(int no);
  SString(size_t no);

  ~SString();

  /** Changes the length of this StringBuffer */
  void setLength(int32_t newLength);

  wchar operator[](int32_t i) const override;
  int32_t length() const override;

  /** Appends to this string buffer @c string */
  SString &append(const String &string, int32_t maxlen = INT32_MAX);
  /** Appends to this string buffer @c string */
  SString &append(const String* string, int32_t maxlen = INT32_MAX);

  /** Appends to this string buffer @c string */
  SString &append(wchar c);

  /** Appends to this string buffer @c string.
      C++ operator+ form.
      You can write: yourcall(StringBuffer("first")+"second"+third);
  */
  SString &operator+(const String &string);
  /** Appends to this string buffer @c string. C++ operator+ form. */
  SString &operator+(const String* string);
  /** Appends to this string buffer @c string. C++ operator+ form. */
  SString &operator+(const char* string);
  /** Appends to this string buffer @c string. C++ operator+= form. */
  SString &operator+=(const String &string);
  /** Appends to this string buffer @c string. C++ operator+= form. */
  SString &operator+=(const char* string);

  SString &operator=(SString const &cstring);
  SString* replace(const String &pattern, const String &newstring) const;
  int compareTo(const SString& str) const;
  int compareTo(const DString& str) const;
  SString(SString &&cstring);
  SString &operator=(SString &&cstring);
protected:

  void construct(const String* cstring, int32_t s, int32_t l);

  std::unique_ptr<wchar[]> wstr;
  int32_t len;
  int32_t alloc;
};

inline SString::SString(SString &&cstring): wstr(std::move(cstring.wstr)),
  len(cstring.len),
  alloc(cstring.alloc)
{
  cstring.wstr = nullptr;
  cstring.alloc = 0;
  cstring.len = 0;
}

inline SString &SString::operator=(SString &&cstring)
{
  if (this != &cstring) {
    wstr = std::move(cstring.wstr);
    alloc = cstring.alloc;
    len = cstring.len;
    cstring.wstr = nullptr;
    cstring.alloc = 0;
    cstring.len = 0;
  }
  return *this;
}

inline int32_t SString::length() const
{
  return len;
}

inline wchar SString::operator[](int32_t i) const
{
  return wstr[i];
}

#include <unordered_map>
namespace std
{
  // Specializations for unordered containers

  template <> struct hash<SString> {
    size_t operator()(const SString &value) const
    {
      return value.hashCode();
    }
  };
  template <> struct equal_to<SString> {
    bool operator()(const SString &u1, const SString &u2) const
    {
      return u1.compareTo(u2) == 0;
    }
  };

} // namespace std

#endif



