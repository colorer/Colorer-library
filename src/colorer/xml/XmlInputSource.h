#ifndef _COLORER_XMLINPUTSOURCE_H_
#define _COLORER_XMLINPUTSOURCE_H_

#include <colorer/Common.h>
#include <xercesc/sax/InputSource.hpp>

const char16_t kJar[] = u"jar:\0";
const char16_t kPercent[] = u"%\0";

class XmlInputSource;

typedef std::unique_ptr<XmlInputSource> uXmlInputSource;

/**
 * @brief Class to creat xercesc::InputSource
 */
class XmlInputSource : public xercesc::InputSource
{
 public:
  /**
   * @brief Statically creates instance of uXmlInputSource object,
   * possibly based on parent source stream.
   * @param base Base stream, used to resolve relative paths.
   * @param path Could be relative file location, absolute file
   */
  static uXmlInputSource newInstance(const XMLCh* path, XmlInputSource* base);

  /**
   * @brief Tries statically create instance of InputSource object,
   * according to passed path string.
   * @param path Could be relative file location, absolute file
   */
  static uXmlInputSource newInstance(const XMLCh* path, const XMLCh* base);
  static uXmlInputSource newInstance(const UnicodeString* path, const UnicodeString* base = nullptr);

  /**
   * @brief Creates inherited InputSource with the same type
   * relatively to the current.
   * @param relPath Relative URI part.
   */
  virtual uXmlInputSource createRelative(const XMLCh* relPath) const = 0;

  virtual xercesc::InputSource* getInputSource()
  {
    return nullptr;
  }

  ~XmlInputSource() override = default;

  static uUnicodeString getAbsolutePath(const UnicodeString* basePath, const UnicodeString* relPath);
  static uUnicodeString getClearPath(const UnicodeString* basePath, const UnicodeString* relPath);

  static bool isUriFile(const UnicodeString* path, const UnicodeString* base = nullptr);

  XmlInputSource(XmlInputSource const&) = delete;
  XmlInputSource& operator=(XmlInputSource const&) = delete;
  XmlInputSource(XmlInputSource&&) = delete;
  XmlInputSource& operator=(XmlInputSource&&) = delete;

 protected:
  XmlInputSource() = default;
};

#endif  //_COLORER_XMLINPUTSOURCE_H_
