#ifndef _COLORER_PARSERFACTORYIMPL_H_
#define _COLORER_PARSERFACTORYIMPL_H_

#include <colorer/HRCParser.h>
#include <colorer/ParserFactory.h>
#include <colorer/TextParser.h>
#include <colorer/handlers/StyledHRDMapper.h>
#include <colorer/handlers/TextHRDMapper.h>
#include <colorer/parsers/HRDNode.h>

class ParserFactory::Impl
{
 public:
  Impl();
  ~Impl();

  void loadCatalog(const UnicodeString* catalog_path);
  HRCParser* getHRCParser() const;
  static TextParser* createTextParser();
  StyledHRDMapper* createStyledMapper(const UnicodeString* classID, const UnicodeString* nameID);
  TextHRDMapper* createTextMapper(const UnicodeString* nameID);

  // old
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
  void parseCatalog(const UnicodeString& catalog_path);

  void loadHrc(const UnicodeString& hrc_path, const UnicodeString* base_path) const;

  uUnicodeString base_catalog_path;
  std::vector<UnicodeString> hrc_locations;
  std::unordered_map<UnicodeString, std::unique_ptr<std::vector<std::unique_ptr<HRDNode>>>> hrd_nodes;

  HRCParser* hrc_parser;
};

#endif  // _COLORER_PARSERFACTORYIMPL_H_
