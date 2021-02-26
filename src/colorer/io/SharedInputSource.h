#ifndef _COLORER_SHAREDINPUTSOURCE_H_
#define _COLORER_SHAREDINPUTSOURCE_H_

#include<colorer/Common.h>
#include<colorer/io/InputSource.h>

/**
 * InputSource class wrapper,
 * allows to manage class instances counter.
 * @ingroup common_io
 */
class SharedInputSource : colorer::InputSource
{

public:

  static SharedInputSource* getInputSource(const UnicodeString* path, colorer::InputSource* base);

  /** Increments reference counter */
  int addref()
  {
    return ++ref_count;
  }

  /** Decrements reference counter */
  int delref()
  {
    if (ref_count == 0) {
      spdlog::error("[SharedInputSource] delref: already zeroed references");
    }
    ref_count--;
    if (ref_count <= 0) {
      delete this;
      return -1;
    }
    return ref_count;
  }

  /**
   * Returns currently opened stream.
   * Opens it, if not yet opened.
   */
  const byte* getStream()
  {
    if (stream == nullptr) {
      stream = openStream();
    }
    return stream;
  }

  [[nodiscard]] const UnicodeString* getLocation() const override
  {
    return is->getLocation();
  }

  const byte* openStream() override
  {
    return is->openStream();
  }

  void closeStream() override
  {
    is->closeStream();
  }

  [[nodiscard]] int length() const override
  {
    return is->length();
  }

private:

  explicit SharedInputSource(colorer::InputSource* source);
  ~SharedInputSource() override;

  static std::unordered_map<UnicodeString, SharedInputSource*>* isHash;

  colorer::InputSource* is;
  const byte* stream;
  int ref_count;
};

#endif

