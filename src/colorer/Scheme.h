#ifndef _COLORER_SCHEME_H_
#define _COLORER_SCHEME_H_

#include <colorer/Common.h>
class FileType;

/** HRC Scheme instance information.
    Used in RegionHandler calls to pass curent region's scheme.
    @ingroup colorer
*/
class Scheme
{
public:
  /** Full qualified schema name.
  */
  [[nodiscard]] virtual const UnicodeString* getName() const = 0;
  /** Returns reference to FileType, this scheme belongs to.
  */
  [[nodiscard]] virtual FileType* getFileType() const = 0;
protected:
  Scheme() = default;
  virtual ~Scheme() = default;
};

#endif


