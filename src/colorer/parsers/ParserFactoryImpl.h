#ifndef _COLORER_PARSERFACTORYIMPL_H_
#define _COLORER_PARSERFACTORYIMPL_H_

#include <colorer/ParserFactory.h>
#include <colorer/HRCParser.h>
#include <colorer/TextParser.h>
#include <colorer/handlers/StyledHRDMapper.h>
#include <colorer/handlers/TextHRDMapper.h>
#include <colorer/parsers/HRDNode.h>

/**
 * Maintains catalog of HRC and HRD references.
 * This class searches and loads <code>catalog.xml</code> file
 * and creates HRCParser, StyledHRDMapper, TextHRDMapper and TextParser instances
 * with information, loaded from specified sources.
 *
 * If no path were passed to it's constructor,
 * it uses the next search order to find 'catalog.xml' file:
 *
 * - win32 systems:
 *   - image_start_dir, image_start_dir\..\
 *   - %COLORER5CATALOG%
 *   - %HOMEDRIVE%%HOMEPATH%\.colorer5catalog
 *
 * - unix/macos systems:
 *   - $COLORER5CATALOG
 *   - $HOME/.colorer5catalog
 *   - $HOMEPATH/.colorer5catalog
 *   - /usr/share/colorer/catalog.xml
 *   - /usr/local/share/colorer/catalog.xml
 *
 * @note
 *   - \%NAME%, \$NAME - Environment variable of the current process.
 *   - image_start_dir - Directory, where current image was started.
 *
 * @ingroup colorer
 */
class ParserFactory::Impl
{
 public:

  Impl();
  ~Impl();

  void loadCatalog(const UnicodeString* catalog_path);
  HRCParser* getHRCParser() const;
  TextParser* createTextParser();
  StyledHRDMapper* createStyledMapper(const UnicodeString* classID, const UnicodeString* nameID);
  TextHRDMapper* createTextMapper(const UnicodeString* nameID);

// old
  static const char* getVersion();

  /**
   * Enumerates all declared hrd classes
   */
  std::vector<UnicodeString> enumHRDClasses();

  /**
   * Enumerates all declared hrd instances of specified class
   */
  std::vector<const HRDNode*> enumHRDInstances(const UnicodeString& classID);

  const HRDNode* getHRDNode(const UnicodeString& classID, const UnicodeString& nameID);

  size_t countHRD(const UnicodeString& classID);

  void addHrd(std::unique_ptr<HRDNode> hrd);

 private:
  UnicodeString searchCatalog() const;
  void getPossibleCatalogPaths(std::vector<UnicodeString>& paths) const;

  void parseCatalog(const UnicodeString& catalog_path);

  void loadHrc(const UnicodeString* hrc_path, const UnicodeString* base_path) const;

  UnicodeString base_catalog_path;
  std::vector<UnicodeString> hrc_locations;
  std::unordered_map<UnicodeString, std::unique_ptr<std::vector<std::unique_ptr<HRDNode>>>> hrd_nodes;

  HRCParser* hrc_parser;
};

#endif  // _COLORER_PARSERFACTORYIMPL_H_
