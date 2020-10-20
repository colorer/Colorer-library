#ifndef _COLORER_PARSERFACTORY_H_
#define _COLORER_PARSERFACTORY_H_

#include <colorer/Common.h>
#include <colorer/Exception.h>
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
 * - Windows systems:
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
class ParserFactory
{
 public:
  /**
   * ParserFactory Constructor.
   * Searches for catalog.xml in the set of predefined locations
   * @throw ParserFactoryException If can't find catalog at any of standard locations.
   */
  ParserFactory();
  ~ParserFactory() = default;

  /**
   * @param catalog_path Path to catalog.xml file. If null,
   *        standard search method is used.
   * @throw ParserFactoryException If can't load specified catalog.
   */
  void loadCatalog(const UnicodeString* catalog_path);
  /**
   * Creates and loads HRCParser instance from catalog.xml file.
   * This method can detect directory entries, and sequentally load their
   * contents into created HRCParser instance.
   * In other cases it uses InputSource#newInstance() method to
   * create input data stream.
   * Only one HRCParser instance is created for each ParserFactory instance.
   */
  [[nodiscard]] HRCParser* getHRCParser() const;

  /**
   * Creates TextParser instance
   */
  TextParser* createTextParser();
  /**
   * Creates RegionMapper instance and loads specified hrd files into it.
   * @param classID Class identifier of loaded hrd instance.
   * @param nameID  Name identifier of loaded hrd instances.
   * @throw ParserFactoryException If method can't find specified pair of
   *         class and name IDs in catalog.xml file
   */
  StyledHRDMapper* createStyledMapper(const UnicodeString* classID, const UnicodeString* nameID);

  /**
   * Creates RegionMapper instance and loads specified hrd files into it.
   * It uses 'text' class by default.
   * @param nameID  Name identifier of loaded hrd instances.
   * @throw ParserFactoryException If method can't find specified pair of
   *         class and name IDs in catalog.xml file
   */
  TextHRDMapper* createTextMapper(const UnicodeString* nameID);

  std::vector<const HRDNode*> enumHRDInstances(const UnicodeString& classID);
  void addHrd(std::unique_ptr<HRDNode> hrd);
  const HRDNode* getHRDNode(const UnicodeString& classID, const UnicodeString& nameID);

  ParserFactory(const ParserFactory&) = delete;
  void operator=(const ParserFactory&) = delete;

 private:
  class Impl;

  std::unique_ptr<Impl> pimpl;
};

/** Exception, thrown by ParserFactory class methods.
    Indicates some (mostly fatal) errors in loading of catalog file (catalog.xml), or in creating parsers objects.
    @ingroup colorer
*/
class ParserFactoryException : public Exception
{
 public:
  explicit ParserFactoryException(const UnicodeString& msg) noexcept : Exception("[ParserFactoryException] " + msg) {}
};

#endif  // _COLORER_PARSERFACTORY_H_
