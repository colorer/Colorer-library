#ifndef _COLORER_REGIONMAPPERIMPL_H_
#define _COLORER_REGIONMAPPERIMPL_H_

#include <colorer/handlers/RegionDefine.h>
#include <colorer/handlers/RegionMapper.h>
#include <colorer/io/Writer.h>
#include <colorer/xml/XmlInputSource.h>
#include <vector>
#include <xercesc/sax/InputSource.hpp>

/** Abstract RegionMapper.
    Stores all region mappings in hashtable and sequental vector.
    @ingroup colorer_handlers
*/
class RegionMapperImpl : public RegionMapper
{
 public:
  RegionMapperImpl() = default;
  ;
  ~RegionMapperImpl() override = default;
  ;

  /** Loads region defines from @c is InputSource
   */
  virtual void loadRegionMappings(XmlInputSource* is) = 0;
  /** Saves all loaded region defines into @c writer.
      Note, that result document would not be equal
      to input one, because there could be multiple input
      documents.
  */
  virtual void saveRegionMappings(Writer* writer) const = 0;
  /** Changes specified region definition to @c rdnew
      @param region Region full qualified name.
      @param rdnew  New region definition to replace old one
  */
  virtual void setRegionDefine(const UnicodeString& region, const RegionDefine* rdnew) = 0;

  /** Enumerates all loaded region defines.
      @return RegionDefine with specified internal index, or null if @c idx is too big
  */
  std::vector<const RegionDefine*> enumerateRegionDefines() const;

  const RegionDefine* getRegionDefine(const Region* region) const override;
  const RegionDefine* getRegionDefine(const UnicodeString& name) const override;

  RegionMapperImpl(const RegionMapperImpl&) = delete;
  void operator=(const RegionMapperImpl&) = delete;

 protected:
  std::unordered_map<UnicodeString, RegionDefine*> regionDefines;
  mutable std::vector<const RegionDefine*> regionDefinesVector;
};

#endif
