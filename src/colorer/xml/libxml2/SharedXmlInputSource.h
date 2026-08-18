#ifndef SHAREDXMLINPUTSOURCE_H
#define SHAREDXMLINPUTSOURCE_H

#include "colorer/Common.h"

/** In-memory copy of a file (typically a zip catalog).
 * Lifetime is owned by the caller via addref/delref — there is no
 * process-wide cache, so a second ParserFactory can reload the same path
 * and see current disk contents.
 */
class SharedXmlInputSource
{
 public:
  static SharedXmlInputSource* getSharedInputSource(const UnicodeString& path);

  /** Increments reference counter */
  int addref();
  /** Decrements reference counter */
  int delref();

  [[nodiscard]]
  int getSize() const;
  [[nodiscard]]
  byte* getSrc() const;

  void open();

  ~SharedXmlInputSource();

  SharedXmlInputSource(SharedXmlInputSource const&) = delete;
  SharedXmlInputSource& operator=(SharedXmlInputSource const&) = delete;
  SharedXmlInputSource(SharedXmlInputSource&&) = delete;
  SharedXmlInputSource& operator=(SharedXmlInputSource&&) = delete;

 private:
  explicit SharedXmlInputSource(const UnicodeString& path);

  int ref_count {1};
  bool is_open {false};
  UnicodeString source_path;
  std::unique_ptr<byte[]> mSrc;
  int mSize {0};
};

#endif  // SHAREDXMLINPUTSOURCE_H
