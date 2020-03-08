#ifndef _COLORER_HRCPARSERPELPERS_H_
#define _COLORER_HRCPARSERPELPERS_H_

#include <memory>
#include <vector>
#include <colorer/cregexp/cregexp.h>
#include <colorer/Scheme.h>
#include <colorer/parsers/SchemeNode.h>


class FileTypeImpl;


/** Scheme storage implementation.
    Manages the vector of SchemeNode's.
    @ingroup colorer_parsers
*/
class SchemeImpl : public Scheme
{
  friend class HRCParserImpl;
  friend class TextParserImpl;
public:
  [[nodiscard]] const UnicodeString* getName() const override
  {
    return schemeName.get();
  }

  [[nodiscard]] FileType* getFileType() const override
  {
    return (FileType*)fileType;
  }


protected:
  uUnicodeString schemeName;
  std::vector<SchemeNode*> nodes;
  FileTypeImpl* fileType;

  explicit SchemeImpl(const UnicodeString* sn)
  {
    schemeName = std::make_unique<UnicodeString>(*sn);
    fileType = nullptr;
  }

  ~SchemeImpl() override
  {
    for (auto it : nodes) {
      delete it;
    }
  }
};

#endif


