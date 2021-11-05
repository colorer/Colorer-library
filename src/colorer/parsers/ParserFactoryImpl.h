#ifndef COLORER_PARSERFACTORYIMPL_H
#define COLORER_PARSERFACTORYIMPL_H

#include "colorer/HrcLibrary.h"
#include "colorer/ParserFactory.h"
#include "colorer/TextParser.h"
#include "colorer/handlers/StyledHRDMapper.h"
#include "colorer/handlers/TextHRDMapper.h"
#include "colorer/parsers/HRDNode.h"

class ParserFactory::Impl
{
 public:
  Impl();
  ~Impl();

  void loadCatalog(const UnicodeString* catalog_path);
  void loadHrcPath(const UnicodeString& location);
  [[nodiscard]] HrcLibrary* getHrcLibrary() const;
  static TextParser* createTextParser();
  StyledHRDMapper* createStyledMapper(const UnicodeString* classID, const UnicodeString* nameID);
  TextHRDMapper* createTextMapper(const UnicodeString* nameID);

  /**
   * Enumerates all declared hrd classes
   */
  [[maybe_unused]] std::vector<UnicodeString> enumHRDClasses();

  /**
   * Enumerates all declared hrd instances of specified class
   */
  std::vector<const HRDNode*> enumHRDInstances(const UnicodeString& classID);

  const HRDNode* getHRDNode(const UnicodeString& classID, const UnicodeString& nameID);

  void addHrd(std::unique_ptr<HRDNode> hrd);

 private:
  void parseCatalog(const UnicodeString& catalog_path);
  void loadHrc(const UnicodeString& hrc_path, const UnicodeString* base_path) const;
  void fillMapper(const UnicodeString* classID, const UnicodeString* nameID, RegionMapper* mapper);

  uUnicodeString base_catalog_path;
  std::vector<UnicodeString> hrc_locations;
  std::unordered_map<UnicodeString, std::unique_ptr<std::vector<std::unique_ptr<HRDNode>>>> hrd_nodes;

  HrcLibrary* hrc_parser;
};

#endif  // COLORER_PARSERFACTORYIMPL_H
