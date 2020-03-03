#ifndef _COLORER_TEXTHRDMAPPER_H_
#define _COLORER_TEXTHRDMAPPER_H_

#include <colorer/handlers/RegionMapperImpl.h>
#include <colorer/handlers/TextRegion.h>
#include <colorer/io/Writer.h>
#include <colorer/xml/XmlInputSource.h>

/** HRD files reader.
    HRD Files format contains mappings of HRC syntax regions into
    text indention information.
    For example, HTML indention (@c stext, @c sback, @c etext, @c eback)
    allows to create colorized HTML code.

    @ingroup colorer_handlers
*/
class TextHRDMapper : public RegionMapperImpl
{
 public:
  TextHRDMapper();
  ~TextHRDMapper() override;

  /**
   * Loads region defines from @c is InputSource
   */
  void loadRegionMappings(XmlInputSource* is) override;

  /**
   * Saves all loaded region defines into @c writer.
   * Note, that result document would not be equal
   * to input one, because there could be multiple input
   * documents.
   */
  void saveRegionMappings(Writer* writer) const override;

  /**
   * Changes specified region definition to @c rdnew
   * @param region Region full qualified name.
   * @param rdnew  New region definition to replace old one
   */
  void setRegionDefine(const UnicodeString& region, const RegionDefine* rdnew) override;
};

#endif
