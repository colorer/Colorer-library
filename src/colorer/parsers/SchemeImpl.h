#ifndef _COLORER_HRCPARSERPELPERS_H_
#define _COLORER_HRCPARSERPELPERS_H_

#include "colorer/Scheme.h"
#include "colorer/cregexp/cregexp.h"
#include "colorer/parsers/SchemeNode.h"
#include <memory>
#include <vector>
#include "colorer/TextParser.h"

class FileType;

/** Scheme storage implementation.
    Manages the vector of SchemeNode's.
    @ingroup colorer_parsers
*/
class SchemeImpl : public Scheme
{
  friend class HrcLibrary;
  friend class TextParser;

 public:
  [[nodiscard]] const UnicodeString* getName() const override
  {
    return schemeName.get();
  }

  [[nodiscard]] FileType* getFileType() const override
  {
    return fileType;
  }

 protected:
  uUnicodeString schemeName;
  std::vector<std::unique_ptr<SchemeNode>> nodes;
  FileType* fileType = nullptr;

  explicit SchemeImpl(const UnicodeString* sn)
  {
    schemeName = std::make_unique<UnicodeString>(*sn);
  }
};

#endif
