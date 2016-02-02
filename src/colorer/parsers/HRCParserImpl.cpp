#include <xercesc/parsers/XercesDOMParser.hpp>
#include <math.h>
#include <stdio.h>
#include <colorer/parsers/helpers/HRCParserHelpers.h>
#include <colorer/parsers/HRCParserImpl.h>
#include <xml/XmlParserErrorHandler.h>
#include <xml/XmlInputSource.h>
#include <xml/BaseEntityResolver.h>
#include <xml/XmlTagDefs.h>
#include <xml/XStr.h>
#include <unicode/UnicodeTools.h>

#define CURRENT_FILE " Current file " << XStr(current_input_source->getInputSource()->getSystemId()) << "."

HRCParserImpl::HRCParserImpl():
  versionName(nullptr), parseProtoType(nullptr), parseType(nullptr), current_input_source(nullptr),
  structureChanged(false), updateStarted(false)
{
  fileTypeHash.reserve(200);
  fileTypeVector.reserve(150);
  regionNamesHash.reserve(1000);
  regionNamesVector.reserve(1000);
  schemeHash.reserve(4000);
}

HRCParserImpl::~HRCParserImpl()
{
  for (auto it : fileTypeHash) {
    delete it.second;
  }

  for (auto it : schemeHash) {
    delete it.second;
  }

  for (auto it : regionNamesVector) {
    delete it;
  }

  for (auto it : schemeEntitiesHash) {
    delete it.second;
  }

  delete versionName;
}

void HRCParserImpl::loadSource(XmlInputSource* is)
{
  if (!is) {
    throw HRCParserException("Can't open stream - 'null' is bad stream.");
  }

  XmlInputSource* istemp = current_input_source;
  current_input_source = is;
  try {
    parseHRC(is);
  } catch (Exception &) {
    current_input_source = istemp;
    throw;
  }
  current_input_source = istemp;
}

void HRCParserImpl::unloadFileType(FileTypeImpl* filetype)
{
  bool loop = true;
  while (loop) {
    loop = false;
    for (auto scheme = schemeHash.begin(); scheme != schemeHash.end(); ++scheme) {
      if (scheme->second->fileType == filetype) {
        schemeHash.erase(scheme);
        loop = true;
        break;
      }
    }
  }
  for (auto ft = fileTypeVector.begin(); ft != fileTypeVector.end(); ++ft) {
    if (*ft == filetype) {
      fileTypeVector.erase(ft);
      break;
    }
  }
  fileTypeHash.erase(filetype->getName());
  delete filetype;
}

void HRCParserImpl::loadFileType(FileType* filetype)
{
  FileTypeImpl* thisType = static_cast<FileTypeImpl*>(filetype);
  if (thisType == nullptr || thisType->type_loaded || thisType->input_source_loading || thisType->load_broken) {
    return;
  }

  thisType->input_source_loading = true;

  try {
    loadSource(thisType->inputSource.get());
  } catch (InputSourceException &e) {
    LOGF(ERRORF, "Can't open source stream: %s", e.what());
    thisType->load_broken = true;
  } catch (HRCParserException &e) {
    LOGF(ERRORF, "%s [%s]", e.what(), XStr(thisType->inputSource->getInputSource()->getSystemId()).get_char());
    thisType->load_broken = true;
  } catch (Exception &e) {
    LOGF(ERRORF, "%s [%s]", e.what(), XStr(thisType->inputSource->getInputSource()->getSystemId()).get_char());
    thisType->load_broken = true;
  } catch (...) {
    LOGF(ERRORF, "Unknown exception while loading %s", XStr(thisType->inputSource->getInputSource()->getSystemId()).get_char());
    thisType->load_broken = true;
  }

  thisType->input_source_loading = false;
}

FileType* HRCParserImpl::chooseFileType(const String* fileName, const String* firstLine, int typeNo)
{
  FileTypeImpl* best = nullptr;
  double max_prior = 0;
  const double DELTA = 1e-6;
  for (int idx = 0; idx < fileTypeVector.size(); idx++) {
    FileTypeImpl* ret = fileTypeVector.at(idx);
    double prior = ret->getPriority(fileName, firstLine);

    if (typeNo > 0 && (prior - max_prior < DELTA)) {
      best = ret;
      typeNo--;
    }
    if (prior - max_prior > DELTA || best == nullptr) {
      best = ret;
      max_prior = prior;
    }
  }
  if (typeNo > 0) {
    return nullptr;
  }
  return best;
}

FileType* HRCParserImpl::getFileType(const String* name)
{
  if (name == nullptr) {
    return nullptr;
  }
  return fileTypeHash.find(name)->second;
}

FileType* HRCParserImpl::enumerateFileTypes(int index)
{
  if (index < fileTypeVector.size()) {
    return fileTypeVector[index];
  }
  return nullptr;
}

size_t HRCParserImpl::getFileTypesCount()
{
  return fileTypeVector.size();
}

size_t HRCParserImpl::getRegionCount()
{
  return regionNamesVector.size();
}

const Region* HRCParserImpl::getRegion(int id)
{
  if (id < 0 || id >= regionNamesVector.size()) {
    return nullptr;
  }
  return regionNamesVector[id];
}

const Region* HRCParserImpl::getRegion(const String* name)
{
  if (name == nullptr) {
    return nullptr;
  }
  return getNCRegion(name, false); // regionNamesHash.get(name);
}

const String* HRCParserImpl::getVersion()
{
  return versionName;
}


// protected methods


void HRCParserImpl::parseHRC(XmlInputSource* is)
{
  LOG(DEBUG) << "begin parse '" << XStr(is->getInputSource()->getSystemId()) << "'";
  xercesc::XercesDOMParser xml_parser;
  XmlParserErrorHandler error_handler;
  BaseEntityResolver resolver;
  xml_parser.setErrorHandler(&error_handler);
  xml_parser.setXMLEntityResolver(&resolver);
  xml_parser.setLoadExternalDTD(false);
  xml_parser.setSkipDTDValidation(true);
  xml_parser.parse(*is->getInputSource());
  if (error_handler.getSawErrors()) {
    throw HRCParserException("Error reading hrc file '" + *XStr(is->getInputSource()->getSystemId()).get_stdstr() + "'");
  }
  xercesc::DOMDocument* doc = xml_parser.getDocument();
  xercesc::DOMElement* root = doc->getDocumentElement();

  if (root && !xercesc::XMLString::equals(root->getNodeName(), hrcTagHrc)) {
    throw HRCParserException("Incorrect hrc-file structure. Main '<hrc>' block not found. Current file " + \
                             *XStr(is->getInputSource()->getSystemId()).get_stdstr());
  }
  if (versionName == nullptr) {
    versionName = new DString(root->getAttribute(hrcHrcAttrVersion));
  }

  bool globalUpdateStarted = false;
  if (!updateStarted) {
    globalUpdateStarted = true;
    updateStarted = true;
  }

  parseHrcBlock(root);

  structureChanged = true;
  if (globalUpdateStarted) {
    updateLinks();
    updateStarted = false;
  }

  LOG(DEBUG) << "end parse '" << XStr(is->getInputSource()->getSystemId()) << "'";
}

void HRCParserImpl::parseHrcBlock(const xercesc::DOMElement* elem)
{
  for (xercesc::DOMNode* node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement* sub_elem = static_cast<xercesc::DOMElement*>(node);
      parseHrcBlockElements(sub_elem);
      continue;
    }
    if (node->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      for (xercesc::DOMNode* sub_node = node->getFirstChild(); sub_node != nullptr; sub_node = sub_node->getNextSibling()) {
        xercesc::DOMElement* sub_elem = static_cast<xercesc::DOMElement*>(sub_node);
        parseHrcBlockElements(sub_elem);
      }
    }
  }
}

void HRCParserImpl::parseHrcBlockElements(const xercesc::DOMElement* elem)
{
  if (elem->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
    if (xercesc::XMLString::equals(elem->getNodeName(), hrcTagPrototype)) {
      addPrototype(elem);
      return;
    }
    if (xercesc::XMLString::equals(elem->getNodeName(), hrcTagPackage)) {
      addPrototype(elem);
      return;
    }
    if (xercesc::XMLString::equals(elem->getNodeName(), hrcTagType)) {
      addType(elem);
      return;
    }
    if (xercesc::XMLString::equals(elem->getNodeName(), hrcTagAnnotation)) {
      // not read anotation
      return;
    }
    LOG(WARNING) << "Unused element '" << XStr(elem->getNodeName()) << "'." << CURRENT_FILE;
  }
}

void HRCParserImpl::addPrototype(const xercesc::DOMElement* elem)
{
  const XMLCh* typeName = elem->getAttribute(hrcPrototypeAttrName);
  const XMLCh* typeGroup = elem->getAttribute(hrcPrototypeAttrGroup);
  const XMLCh* typeDescription = elem->getAttribute(hrcPrototypeAttrDescription);
  if (*typeName == '\0') {
    LOG(ERROR) << "unnamed prototype";
    return;
  }
  if (typeDescription == nullptr) {
    typeDescription = typeName;
  }
  FileTypeImpl* f = nullptr;
  DString tname = DString(typeName);
  auto ft = fileTypeHash.find(&tname);
  if (ft != fileTypeHash.end()) {
    f = ft->second;
  }
  if (f != nullptr) {
    unloadFileType(f);
    LOGF(WARNING, "Duplicate prototype '%s'", tname.getChars());
    //  return;
  }
  FileTypeImpl* type = new FileTypeImpl(this);
  type->name.reset(new SString(DString(typeName)));
  type->description.reset(new SString(DString(typeDescription)));
  if (typeGroup != nullptr) {
    type->group.reset(new SString(DString(typeGroup)));
  }
  if (xercesc::XMLString::equals(elem->getNodeName(), hrcTagPackage)) {
    type->isPackage = true;
  }
  parseProtoType = type;
  parsePrototypeBlock(elem);

  type->protoLoaded = true;
  std::pair<SString, FileTypeImpl*> pp(type->getName(), type);
  fileTypeHash.emplace(pp);

  if (!type->isPackage) {
    fileTypeVector.push_back(type);
  }
}
void HRCParserImpl::parsePrototypeBlock(const xercesc::DOMElement* elem)
{
  for (xercesc::DOMNode* node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement* subelem = static_cast<xercesc::DOMElement*>(node);
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagLocation)) {
        addPrototypeLocation(subelem);
        continue;
      }
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagFilename) ||
          xercesc::XMLString::equals(subelem->getNodeName(), hrcTagFirstline)) {
        addPrototypeDetectParam(subelem);
        continue;
      }
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagParametrs)) {
        addPrototypeParameters(subelem);
        continue;
      }
    }
  }
}

void HRCParserImpl::addPrototypeLocation(const xercesc::DOMElement* elem)
{
  const XMLCh* locationLink = elem->getAttribute(hrcLocationAttrLink);
  if (*locationLink == '\0') {
    LOGF(ERROR, "Bad 'location' link attribute in prototype '%s'", parseProtoType->name->getChars());
    return;
  }
  parseProtoType->inputSource = XmlInputSource::newInstance(locationLink, current_input_source);
}

void HRCParserImpl::addPrototypeDetectParam(const xercesc::DOMElement* elem)
{
  if (elem->getFirstChild() == nullptr || elem->getFirstChild()->getNodeType() != xercesc::DOMNode::TEXT_NODE) {
    LOGF(WARNING, "Bad '%s' element in prototype '%s'", XStr(elem->getNodeName()).get_char(), parseProtoType->name->getChars());
    return;
  }
  const XMLCh* match = ((xercesc::DOMText*)elem->getFirstChild())->getData();
  DString dmatch = DString(match);
  CRegExp* matchRE = new CRegExp(&dmatch);
  matchRE->setPositionMoves(true);
  if (!matchRE->isOk()) {
    LOGF(WARNING, "Fault compiling chooser RE '%s' in prototype '%s'", dmatch.getChars(), parseProtoType->name->getChars());
    delete matchRE;
    return;
  }
  int ctype = xercesc::XMLString::equals(elem->getNodeName() , hrcTagFilename) ? 0 : 1;
  double prior = ctype ? 1 : 2;
  DString weight= DString(elem->getAttribute(hrcFilenameAttrWeight));
  UnicodeTools::getNumber(&weight, &prior);
  FileTypeChooser* ftc = new FileTypeChooser(ctype, prior, matchRE);
  parseProtoType->chooserVector.push_back(ftc);
}

void HRCParserImpl::addPrototypeParameters(const xercesc::DOMElement* elem)
{
  for (xercesc::DOMNode* node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement* subelem = static_cast<xercesc::DOMElement*>(node);
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagParam)) {
        const XMLCh* name = subelem->getAttribute(hrcParamAttrName);
        const XMLCh* value = subelem->getAttribute(hrcParamAttrValue);
        const XMLCh* descr = subelem->getAttribute(hrcParamAttrDescription);
        if (*name == '\0' || *value == '\0') {
          LOGF(WARNING, "Bad parameter in prototype '%s'", parseProtoType->name->getChars());
          continue;
        }
        DString d_name = DString(name);
        TypeParameter* tp = parseProtoType->addParam(&d_name);
        tp->default_value = new SString(DString(value));
        if (*descr != '\0') {
          tp->description = new SString(DString(descr));
        }
      }
      continue;
    }
    if (node->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      addPrototypeParameters(static_cast<xercesc::DOMElement*>(node));
    }
  }
}

void HRCParserImpl::addType(const xercesc::DOMElement* elem)
{
  const XMLCh* typeName = elem->getAttribute(hrcTypeAttrName);

  if (*typeName == '\0') {
    LOG(ERROR) << "Unnamed type found";
    return;
  }
  DString d_name = DString(typeName);
  auto type_ref = fileTypeHash.find(&d_name);
  if (type_ref == fileTypeHash.end()) {
    LOGF(ERROR,"type '%s' without prototype", d_name.getChars());
    return;
  }
  FileTypeImpl* type = type_ref->second;
  if (type->type_loaded) {
    LOGF(WARNING, "type '%s' is already loaded", XStr(typeName).get_char());
    return;
  }
  type->type_loaded = true;

  FileTypeImpl* o_parseType = parseType;
  parseType = type;

  parseTypeBlock(elem);

  String* baseSchemeName = qualifyOwnName(type->getName());
  if (baseSchemeName != nullptr) {
    auto sh = schemeHash.find(baseSchemeName);
    type->baseScheme = sh == schemeHash.end() ? nullptr : sh->second;
  }
  delete baseSchemeName;
  if (type->baseScheme == nullptr && !type->isPackage) {
    LOGF(WARNING, "type '%s' has no default scheme", XStr(typeName).get_char());
  }
  type->loadDone = true;
  parseType = o_parseType;
}

void HRCParserImpl::parseTypeBlock(const xercesc::DOMElement* elem)
{
  for (xercesc::DOMNode* node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement* subelem = static_cast<xercesc::DOMElement*>(node);
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagRegion)) {
        addTypeRegion(subelem);
        continue;
      }
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagEntity)) {
        addTypeEntity(subelem);
        continue;
      }
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagImport)) {
        addTypeImport(subelem);
        continue;
      }
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagScheme)) {
        addScheme(subelem);
        continue;
      }
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagAnnotation)) {
        // not read anotation
        continue;
      }
    }
    // случай entity ссылки на другой файл.
    if (node->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      parseTypeBlock(static_cast<xercesc::DOMElement*>(node));
    }
  }
}

void HRCParserImpl::addTypeRegion(const xercesc::DOMElement* elem)
{
  const XMLCh* regionName = elem->getAttribute(hrcRegionAttrName);
  const XMLCh* regionParent = elem->getAttribute(hrcRegionAttrParent);
  const XMLCh* regionDescr = elem->getAttribute(hrcRegionAttrDescription);
  if (*regionName == '\0') {
    LOG(ERROR) << "No 'name' attribute in <region> element";
    return;
  }
  DString d_region = DString(regionName);
  String* qname1 = qualifyOwnName(&d_region);
  if (qname1 == nullptr) {
    return;
  }
  DString d_regionparent = DString(regionParent);
  String* qname2 = qualifyForeignName(*regionParent != '\0' ? &d_regionparent : nullptr, QNT_DEFINE, true);
  if (regionNamesHash.find(qname1) != regionNamesHash.end()) {
    LOGF(WARNING, "Duplicate region '%s' definition in type '%s'", qname1->getChars(), parseType->getName()->getChars());
    delete qname1;
    delete qname2;
    return;
  }

  DString regiondescr = DString(regionDescr);
  const Region* region = new Region(qname1, &regiondescr, getRegion(qname2), (int)regionNamesVector.size());
  regionNamesVector.push_back(region);
  std::pair<SString, const Region*> pp(qname1, region);
  regionNamesHash.emplace(pp);

  delete qname1;
  delete qname2;
}

void HRCParserImpl::addTypeEntity(const xercesc::DOMElement* elem)
{
  const XMLCh* entityName  = elem->getAttribute(hrcEntityAttrName);
  const XMLCh* entityValue = elem->getAttribute(hrcEntityAttrValue);
  if (*entityName == '\0' || elem->getAttributeNode(hrcEntityAttrValue) == nullptr) {
    LOG(ERROR) << "Bad entity attributes";
    return;
  }
  DString dentityName= DString(entityName);
  DString dentityValue= DString(entityValue);
  String* qname1 = qualifyOwnName(&dentityName);
  String* qname2 = useEntities(&dentityValue);
  if (qname1 != nullptr && qname2 != nullptr) {
    std::pair<SString, String*> pp(qname1, qname2);
    schemeEntitiesHash.emplace(pp);
    delete qname1;
  }
}

void HRCParserImpl::addTypeImport(const xercesc::DOMElement* elem)
{
  const XMLCh* typeParam = elem->getAttribute(hrcImportAttrType);
  DString typeparam = DString(typeParam);
  if (*typeParam == '\0' || fileTypeHash.find(&typeparam) == fileTypeHash.end()) {
    LOGF(ERROR, "Import with bad '%s' attribute in type '%s'", typeparam.getChars(), parseType->name->getChars());
    return;
  }
  parseType->importVector.push_back(new SString(DString(typeParam)));
}

void HRCParserImpl::addScheme(const xercesc::DOMElement* elem)
{
  const XMLCh* schemeName = elem->getAttribute(hrcSchemeAttrName);
  DString dschemeName = DString(schemeName);
  String* qSchemeName = qualifyOwnName(*schemeName != '\0' ? &dschemeName : nullptr);
  if (qSchemeName == nullptr) {
    LOGF(ERROR, "bad scheme name in type '%s'", parseType->getName()->getChars());
    return;
  }
  if (schemeHash.find(qSchemeName) != schemeHash.end() ||
      disabledSchemes.find(qSchemeName) != disabledSchemes.end()) {
    LOGF(ERROR, "duplicate scheme name '%s'", qSchemeName->getChars());
    delete qSchemeName;
    return;
  }

  SchemeImpl* scheme = new SchemeImpl(qSchemeName);
  delete qSchemeName;
  scheme->fileType = parseType;

  std::pair<SString, SchemeImpl*> pp(scheme->schemeName, scheme);
  schemeHash.emplace(pp);
  const XMLCh* condIf = elem->getAttribute(hrcSchemeAttrIf);
  const XMLCh* condUnless = elem->getAttribute(hrcSchemeAttrUnless);
  const String* p1 = parseType->getParamValue(DString(condIf));
  const String* p2 = parseType->getParamValue(DString(condUnless));
  if ((*condIf != '\0' && !DString("true").equals(p1)) ||
      (*condUnless != '\0' && DString("true").equals(p2))) {
    //disabledSchemes.put(scheme->schemeName, 1);
    return;
  }
  parseSchemeBlock(scheme, elem);
}

void HRCParserImpl::parseSchemeBlock(SchemeImpl* scheme, const xercesc::DOMElement* elem)
{
  for (xercesc::DOMNode* node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement* subelem = static_cast<xercesc::DOMElement*>(node);
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagInherit)) {
        addSchemeInherit(scheme, subelem);
        continue;
      }
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagRegexp)) {
        addSchemeRegexp(scheme, subelem);
        continue;
      }
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagBlock)) {
        addSchemeBlock(scheme, subelem);
        continue;
      }
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagKeywords)) {
        addSchemeKeywords(scheme, subelem);
        continue;
      }
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagAnnotation)) {
        // not read anotation
        continue;
      }
    }
    if (node->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      parseSchemeBlock(scheme, static_cast<xercesc::DOMElement*>(node));
    }
  }
}

void HRCParserImpl::addSchemeInherit(SchemeImpl* scheme, const xercesc::DOMElement* elem)
{
  const XMLCh* nqSchemeName = elem->getAttribute(hrcInheritAttrScheme);
  if (*nqSchemeName == '\0') {
    LOGF(ERROR, "empty scheme name in inheritance operator in scheme '%s'", scheme->schemeName->getChars());
    return;
  }
  SchemeNode* scheme_node = new SchemeNode();
  scheme_node->type = SNT_INHERIT;
  scheme_node->schemeName = new SString(DString(nqSchemeName));
  DString dnqSchemeName = DString(nqSchemeName);
  String* schemeName = qualifyForeignName(&dnqSchemeName, QNT_SCHEME, false);
  if (schemeName == nullptr) {
    //        if (errorHandler != null) errorHandler->warning(StringBuffer("forward inheritance of '")+nqSchemeName+"'. possible inherit loop with '"+scheme->schemeName+"'");
    //        delete next;
    //        continue;
  } else {
    scheme_node->scheme = schemeHash.find(schemeName)->second;
  }
  if (schemeName != nullptr) {
    delete scheme_node->schemeName;
    scheme_node->schemeName = schemeName;
  }

  for (xercesc::DOMNode* node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement* subelem = static_cast<xercesc::DOMElement*>(node);
      if (xercesc::XMLString::equals(subelem->getNodeName() , hrcTagVirtual)) {
        const XMLCh* x_schemeName = subelem->getAttribute(hrcVirtualAttrScheme);
        const XMLCh* x_substName = subelem->getAttribute(hrcVirtualAttrSubstScheme);
        if (*x_schemeName == '\0' || *x_substName == '\0') {
          LOGF(ERROR, "bad virtualize attributes in scheme '%s'", scheme->schemeName->getChars());
          continue;
        }
        DString d_schemeName = DString(x_schemeName);
        DString d_substName = DString(x_substName);
        scheme_node->virtualEntryVector.push_back(new VirtualEntry(&d_schemeName, &d_substName));
      }
    }
  }
  scheme->nodes.push_back(scheme_node);
}

void HRCParserImpl::addSchemeRegexp(SchemeImpl* scheme, const xercesc::DOMElement* elem)
{
  const XMLCh* matchParam = elem->getAttribute(hrcRegexpAttrMatch);
  if (*matchParam == '\0') {
    for (xercesc::DOMNode* child = elem->getFirstChild(); child != nullptr; child = child->getNextSibling()) {
      if (child->getNodeType() == xercesc::DOMNode::CDATA_SECTION_NODE) {
        matchParam = ((xercesc::DOMCDATASection*)child)->getData();
        break;
      }
      if (child->getNodeType() == xercesc::DOMNode::TEXT_NODE) {
        const XMLCh* matchParam1;
        matchParam1 = ((xercesc::DOMText*)child)->getData();
        xercesc::XMLString::trim((XMLCh*)matchParam1);
        if (*matchParam1 != '\0') {
          matchParam = matchParam1;
          break;
        }
      }
    }
  }
  if (matchParam == nullptr) {
    LOGF(ERROR, "no 'match' in regexp in scheme '%s'", scheme->schemeName->getChars());
    return;
  }
  DString dmatchParam = DString(matchParam);
  String* entMatchParam = useEntities(&dmatchParam);
  SchemeNode* scheme_node = new SchemeNode();
  DString dhrcRegexpAttrPriority = DString(elem->getAttribute(hrcRegexpAttrPriority));
  scheme_node->lowPriority = DString("low").equals(&dhrcRegexpAttrPriority);
  scheme_node->type = SNT_RE;
  scheme_node->start = new CRegExp(entMatchParam);
  if (!scheme_node->start || !scheme_node->start->isOk())
    LOGF(ERROR, "fault compiling regexp '%s' in scheme '%s'", entMatchParam->getChars(), scheme->schemeName->getChars());
  delete entMatchParam;
  scheme_node->start->setPositionMoves(false);
  scheme_node->end = 0;

  loadRegions(scheme_node, elem, true);
  if (scheme_node->region) {
    scheme_node->regions[0] = scheme_node->region;
  }

  scheme->nodes.push_back(scheme_node);
}

void HRCParserImpl::addSchemeBlock(SchemeImpl* scheme, const xercesc::DOMElement* elem)
{
  const XMLCh* sParam = elem->getAttribute(hrcBlockAttrStart);
  const XMLCh* eParam = elem->getAttribute(hrcBlockAttrEnd);

  xercesc::DOMElement* eStart = NULL, *eEnd = NULL;

  for (xercesc::DOMNode* blkn = elem->getFirstChild(); blkn && !(*eParam != '\0' && *sParam != '\0'); blkn = blkn->getNextSibling()) {
    xercesc::DOMElement* blkel;
    if (blkn->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      blkel = static_cast<xercesc::DOMElement*>(blkn);
    } else {
      continue;
    }

    const XMLCh* p = '\0';
    if (blkel->getAttributeNode(hrcBlockAttrMatch)) {
      p = blkel->getAttribute(hrcBlockAttrMatch);
    } else {
      for (xercesc::DOMNode* child = blkel->getFirstChild(); child != nullptr; child = child->getNextSibling()) {
        if (child->getNodeType() == xercesc::DOMNode::CDATA_SECTION_NODE) {
          p = ((xercesc::DOMCDATASection*)child)->getData();
          break;
        }
        if (child->getNodeType() == xercesc::DOMNode::TEXT_NODE) {
          const XMLCh* p1;
          p1 = ((xercesc::DOMText*)child)->getData();
          xercesc::XMLString::trim((XMLCh*)p1);
          if (*p1 != '\0') {
            p = p1;
            break;
          }
        }
      }
    }

    if (xercesc::XMLString::equals(blkel->getNodeName() , hrcBlockAttrStart)) {
      sParam = p;
      eStart = blkel;
    }
    if (xercesc::XMLString::equals(blkel->getNodeName() , hrcBlockAttrEnd)) {
      eParam = p;
      eEnd = blkel;
    }
  }

  String* startParam;
  String* endParam;
  DString dsParam = DString(sParam);
  if (!(startParam = useEntities(&dsParam))) {
    LOGF(ERROR, "'start' block attribute not found in scheme '%s'", scheme->schemeName->getChars());
    delete startParam;
    return;
  }
    DString deParam = DString(eParam);
  if (!(endParam = useEntities(&deParam))) {
    LOGF(ERROR, "'end' block attribute not found in scheme '%s'", scheme->schemeName->getChars());
    delete startParam;
    delete endParam;
    return;
  }
  const XMLCh* schemeName = elem->getAttribute(hrcBlockAttrScheme);
  if (*schemeName == '\0') {
    LOGF(ERROR, "block with bad scheme attribute in scheme '%s'", scheme->schemeName->getChars());
    delete startParam;
    delete endParam;
    return;
  }
  SchemeNode* scheme_node = new SchemeNode();
  scheme_node->schemeName = new SString(DString(schemeName));
  DString attr_pr= DString(elem->getAttribute(hrcBlockAttrPriority));
  DString attr_cpr= DString(elem->getAttribute(hrcBlockAttrContentPriority));
  DString attr_ireg= DString(elem->getAttribute(hrcBlockAttrInnerRegion));
  scheme_node->lowPriority = DString("low").equals(&attr_pr);
  scheme_node->lowContentPriority = DString("low").equals(&attr_cpr);
  scheme_node->innerRegion = DString("yes").equals(&attr_ireg);
  scheme_node->type = SNT_SCHEME;
  scheme_node->start = new CRegExp(startParam);
  scheme_node->start->setPositionMoves(false);
  if (!scheme_node->start->isOk()) {
    LOGF(ERROR, "fault compiling regexp '%s' in scheme '%s'", startParam->getChars(), scheme->schemeName->getChars());
  }
  scheme_node->end = new CRegExp();
  scheme_node->end->setPositionMoves(true);
  scheme_node->end->setBackRE(scheme_node->start);
  scheme_node->end->setRE(endParam);
  if (!scheme_node->end->isOk()) {
    LOGF(ERROR, "fault compiling regexp '%s' in scheme '%s'", endParam->getChars(), scheme->schemeName->getChars());
  }
  delete startParam;
  delete endParam;

  // !! EE
  loadBlockRegions(scheme_node, elem);
  loadRegions(scheme_node, eStart, true);
  loadRegions(scheme_node, eEnd, false);
  scheme->nodes.push_back(scheme_node);
}

void HRCParserImpl::addSchemeKeywords(SchemeImpl* scheme, const xercesc::DOMElement* elem)
{
  SchemeNode* scheme_node = new SchemeNode();
  DString dhrcKeywordsAttrIgnorecase= DString(elem->getAttribute(hrcKeywordsAttrIgnorecase));
  DString dhrcKeywordsAttrPriority= DString(elem->getAttribute(hrcKeywordsAttrPriority));
  bool isCase = !DString("yes").equals(&dhrcKeywordsAttrIgnorecase);
  scheme_node->lowPriority = !DString("normal").equals(&dhrcKeywordsAttrPriority);
  const Region* brgn = getNCRegion(elem, DString("region"));
  if (brgn == nullptr) {
    return;
  }
  const XMLCh* worddiv = elem->getAttribute(hrcKeywordsAttrWorddiv);

  scheme_node->worddiv = nullptr;
  if (*worddiv != '\0') {
    DString dworddiv= DString(worddiv);
    String* entWordDiv = useEntities(&dworddiv);
    scheme_node->worddiv = CharacterClass::createCharClass(*entWordDiv, 0, nullptr);
    if (scheme_node->worddiv == nullptr) {
      LOGF(ERROR, "fault compiling worddiv regexp '%s' in scheme '%s'", entWordDiv->getChars(), scheme->schemeName->getChars());
    }
    delete entWordDiv;
  }

  scheme_node->kwList = new KeywordList;
  scheme_node->kwList->num = getSchemeKeywordsCount(elem);

  scheme_node->kwList->kwList = new KeywordInfo[scheme_node->kwList->num];
  memset(scheme_node->kwList->kwList , 0, sizeof(KeywordInfo)*scheme_node->kwList->num);
  scheme_node->kwList->num = 0;
  scheme_node->kwList->matchCase = isCase;
  scheme_node->type = SNT_KEYWORDS;

  for (xercesc::DOMNode* keywrd = elem->getFirstChild(); keywrd; keywrd = keywrd->getNextSibling()) {
    if (keywrd->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      addKeyword(scheme_node, brgn, static_cast<xercesc::DOMElement*>(keywrd));
      continue;
    }
    if (keywrd->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      for (xercesc::DOMNode* keywrd2 = keywrd->getFirstChild(); keywrd2; keywrd2 = keywrd2->getNextSibling()) {
        if (keywrd2->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
          addKeyword(scheme_node, brgn, static_cast<xercesc::DOMElement*>(keywrd2));
        }
      }
    }
  }
  scheme_node->kwList->sortList();
  scheme_node->kwList->substrIndex();
  scheme->nodes.push_back(scheme_node);
}

void HRCParserImpl::addKeyword(SchemeNode* scheme_node, const Region* brgn, const xercesc::DOMElement* elem)
{
  int type = 0;
  if (xercesc::XMLString::equals(elem->getNodeName() , hrcTagWord)) {
    type = 1;
  }
  if (xercesc::XMLString::equals(elem->getNodeName() , hrcTagSymb)) {
    type = 2;
  }
  if (!type) {
    return;
  }
  const XMLCh* param = elem->getAttribute(hrcWordAttrName);
  if (*param == '\0') {
    return;
  }

  const Region* rgn = brgn;
  const XMLCh* reg = elem->getAttribute(hrcWordAttrRegion);
  if (*reg != '\0') {
    rgn = getNCRegion(elem, DString(hrcWordAttrRegion));
  }

  int pos = scheme_node->kwList->num;
  scheme_node->kwList->kwList[pos].keyword = new SString(DString(param));
  scheme_node->kwList->kwList[pos].region = rgn;
  scheme_node->kwList->kwList[pos].isSymbol = (type == 2);
  scheme_node->kwList->kwList[pos].ssShorter = -1;
  scheme_node->kwList->firstChar->addChar(param[0]);
  if (!scheme_node->kwList->matchCase) {
    scheme_node->kwList->firstChar->addChar(Character::toLowerCase(param[0]));
    scheme_node->kwList->firstChar->addChar(Character::toUpperCase(param[0]));
    scheme_node->kwList->firstChar->addChar(Character::toTitleCase(param[0]));
  }
  scheme_node->kwList->num++;
  if (scheme_node->kwList->minKeywordLength > scheme_node->kwList->kwList[pos].keyword->length()) {
    scheme_node->kwList->minKeywordLength = scheme_node->kwList->kwList[pos].keyword->length();
  }
}

int HRCParserImpl::getSchemeKeywordsCount(const xercesc::DOMElement* elem)
{
  int result = 0;
  for (xercesc::DOMNode* keywrd_count = elem->getFirstChild(); keywrd_count; keywrd_count = keywrd_count->getNextSibling()) {
    if (keywrd_count->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      if (xercesc::XMLString::equals(keywrd_count->getNodeName() , hrcTagWord) ||
          xercesc::XMLString::equals(keywrd_count->getNodeName() , hrcTagSymb)) {
        result++;
      }
      continue;
    }
    if (keywrd_count->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      result += getSchemeKeywordsCount(static_cast<xercesc::DOMElement*>(keywrd_count));
    }
  }
  return result;
}

void HRCParserImpl::loadRegions(SchemeNode* node, const xercesc::DOMElement* el, bool st)
{
  static char rg_tmpl[8] = "region\0";

  if (el) {
    if (node->region == nullptr) {
      node->region = getNCRegion(el, DString("region"));
    }

    for (int i = 0; i < REGIONS_NUM; i++) {
      rg_tmpl[6] = (i < 0xA ? i : i + 7 + 32) + '0';

      if (st) {
        node->regions[i] = getNCRegion(el, DString(rg_tmpl));
      } else {
        node->regione[i] = getNCRegion(el, DString(rg_tmpl));
      }
    }
  }

  for (int i = 0; i < NAMED_REGIONS_NUM; i++) {
    if (st) {
      node->regionsn[i] = getNCRegion(node->start->getBracketName(i), false);
    } else {
      node->regionen[i] = getNCRegion(node->end->getBracketName(i), false);
    }
  }
}

void HRCParserImpl::loadBlockRegions(SchemeNode* node, const xercesc::DOMElement* el)
{
  int i;
  static char rg_tmpl[9] = "region\0\0";

  node->region = getNCRegion(el, DString("region"));
  for (i = 0; i < REGIONS_NUM; i++) {
    rg_tmpl[6] = '0';
    rg_tmpl[7] = (i < 0xA ? i : i + 7 + 32) + '0';
    node->regions[i] = getNCRegion(el, DString(rg_tmpl));
    rg_tmpl[6] = '1';
    node->regione[i] = getNCRegion(el, DString(rg_tmpl));
  }
}

void HRCParserImpl::updateLinks()
{
  while (structureChanged) {
    structureChanged = false;
    for (auto scheme_it = schemeHash.begin(); scheme_it != schemeHash.end(); ++scheme_it) {
      SchemeImpl* scheme = scheme_it->second;
      if (!scheme->fileType->loadDone) {
        continue;
      }
      FileTypeImpl* old_parseType = parseType;
      parseType = scheme->fileType;
      for (int sni = 0; sni < scheme->nodes.size(); sni++) {
        SchemeNode* snode = scheme->nodes.at(sni);
        if (snode->schemeName != nullptr && (snode->type == SNT_SCHEME || snode->type == SNT_INHERIT) && snode->scheme == nullptr) {
          String* schemeName = qualifyForeignName(snode->schemeName, QNT_SCHEME, true);
          if (schemeName != nullptr) {
            snode->scheme = schemeHash.find(schemeName)->second;
          } else {
            LOGF(ERROR, "cannot resolve scheme name '%s' in scheme '%s'", snode->schemeName->getChars(), scheme->schemeName->getChars());
          }
          delete schemeName;
          delete snode->schemeName;
          snode->schemeName = nullptr;
        }
        if (snode->type == SNT_INHERIT) {
          for (int vti = 0; vti < snode->virtualEntryVector.size(); vti++) {
            VirtualEntry* vt = snode->virtualEntryVector.at(vti);
            if (vt->virtScheme == nullptr && vt->virtSchemeName != nullptr) {
              String* vsn = qualifyForeignName(vt->virtSchemeName, QNT_SCHEME, true);
              if (vsn) {
                vt->virtScheme = schemeHash.find(vsn)->second;
              } else {
                LOGF(ERROR, "cannot virtualize scheme '%s' in scheme '%s'", vt->virtSchemeName->getChars(), scheme->schemeName->getChars());
              }
              delete vsn;
              delete vt->virtSchemeName;
              vt->virtSchemeName = nullptr;
            }
            if (vt->substScheme == nullptr && vt->substSchemeName != nullptr) {
              String* vsn = qualifyForeignName(vt->substSchemeName, QNT_SCHEME, true);
              if (vsn) {
                vt->substScheme = schemeHash.find(vsn)->second;
              }
              else {
                LOGF(ERROR, "cannot virtualize using subst-scheme scheme '%s' in scheme '%s'", vt->substSchemeName->getChars(), scheme->schemeName->getChars());
              }
              delete vsn;
              delete vt->substSchemeName;
              vt->substSchemeName = nullptr;
            }
          }
        }
      }
      parseType = old_parseType;
      if (structureChanged) {
        break;
      }
    }
  }
}

String* HRCParserImpl::qualifyOwnName(const String* name)
{
  if (name == nullptr) {
    return nullptr;
  }
  int colon = name->indexOf(':');
  if (colon != -1) {
    if (parseType && DString(name, 0, colon) != *parseType->name) {
      LOGF(ERROR, "type name qualifer in '%s' doesn't match type '%s'", name->getChars(), parseType->name->getChars());
      return nullptr;
    } else {
      return new SString(name);
    }
  } else {
    if (parseType == nullptr) {
      return nullptr;
    }
    StringBuffer* sbuf = new StringBuffer(parseType->getName());
    sbuf->append(DString(":")).append(name);
    return sbuf;
  }
}

bool HRCParserImpl::checkNameExist(const String* name, FileTypeImpl* parseType, QualifyNameType qntype, bool logErrors)
{
  if (qntype == QNT_DEFINE && regionNamesHash.find(name) == regionNamesHash.end()) {
    if (logErrors)
      LOGF(ERROR, "region '%s', referenced in type '%s', is not defined", name->getChars(), parseType->name->getChars());
    return false;
  } else if (qntype == QNT_ENTITY && schemeEntitiesHash.find(name) == schemeEntitiesHash.end()) {
    if (logErrors)
      LOGF(ERROR, "entity '%s', referenced in type '%s', is not defined", name->getChars(), parseType->name->getChars());
    return false;
  } else if (qntype == QNT_SCHEME && schemeHash.find(name) == schemeHash.end()) {
    if (logErrors)
      LOGF(ERROR, "scheme '%s', referenced in type '%s', is not defined", name->getChars(), parseType->name->getChars());
    return false;
  }
  return true;
}

String* HRCParserImpl::qualifyForeignName(const String* name, QualifyNameType qntype, bool logErrors)
{
  if (name == nullptr) {
    return nullptr;
  }
  int colon = name->indexOf(':');
  if (colon != -1) { // qualified name
    DString prefix(name, 0, colon);
    auto ft = fileTypeHash.find(&prefix);
    FileTypeImpl* prefType = nullptr;
    if (ft != fileTypeHash.end()) {
      prefType = ft->second;
    }

    if (prefType == nullptr) {
      if (logErrors) {
        LOGF(ERROR, "type name qualifer in '%s' doesn't match any type", name->getChars());
      }
      return nullptr;
    } else if (!prefType->type_loaded) {
      loadFileType(prefType);
    }
    if (prefType == parseType || prefType->type_loaded) {
      return checkNameExist(name, prefType, qntype, logErrors) ? (new SString(name)) : nullptr;
    }
  } else { // unqualified name
    for (int idx = -1; parseType != nullptr && idx < static_cast<int>(parseType->importVector.size()); idx++) {
      const String* tname = parseType->getName();
      if (idx > -1) {
        tname = parseType->importVector.at(idx);
      }
      FileTypeImpl* importer = fileTypeHash.find(tname)->second;
      if (!importer->type_loaded) {
        loadFileType(importer);
      }

      StringBuffer* qname = new StringBuffer(tname);
      qname->append(DString(":")).append(name);
      if (checkNameExist(qname, importer, qntype, false)) {
        return qname;
      }
      delete qname;
    }
    if (logErrors) {
      LOGF(ERROR, "unqualified name '%s' doesn't belong to any imported type [%s]", name->getChars(), XStr(current_input_source->getInputSource()->getSystemId()).get_char());
    }
  }
  return nullptr;
}

String* HRCParserImpl::useEntities(const String* name)
{
  int copypos = 0;
  int epos = 0;

  if (!name) {
    return nullptr;
  }
  StringBuffer* newname = new StringBuffer();

  while (true) {
    epos = name->indexOf('%', epos);
    if (epos == -1) {
      epos = name->length();
      break;
    }
    if (epos && (*name)[epos - 1] == '\\') {
      epos++;
      continue;
    }
    int elpos = name->indexOf(';', epos);
    if (elpos == -1) {
      epos = name->length();
      break;
    }
    DString enname(name, epos + 1, elpos - epos - 1);

    String* qEnName = qualifyForeignName(&enname, QNT_ENTITY, true);
    const String* enval = nullptr;
    if (qEnName != nullptr) {
      enval = schemeEntitiesHash.find(qEnName)->second;
      delete qEnName;
    }
    if (enval == nullptr) {
      epos++;
      continue;
    }
    newname->append(DString(name, copypos, epos - copypos));
    newname->append(enval);
    epos = elpos + 1;
    copypos = epos;
  }
  if (epos > copypos) {
    newname->append(DString(name, copypos, epos - copypos));
  }
  return newname;
}

const Region* HRCParserImpl::getNCRegion(const String* name, bool logErrors)
{
  if (name == nullptr) {
    return nullptr;
  }
  const Region* reg;
  String* qname = qualifyForeignName(name, QNT_DEFINE, logErrors);
  if (qname == nullptr) {
    return nullptr;
  }
  auto reg_ = regionNamesHash.find(qname);
  if (reg_ != regionNamesHash.end()) {
    reg = reg_->second;
  } else {
    reg = nullptr;
  }

  delete qname;
  /** Check for 'default' region request.
      Regions with this name are always transparent
  */
  if (reg != nullptr) {
    const String* s_name = reg->getName();
    int idx = s_name->indexOf(DString(":default"));
    if (idx != -1  && idx + 8 == s_name->length()) {
      return nullptr;
    }
  }
  return reg;
}

const Region* HRCParserImpl::getNCRegion(const xercesc::DOMElement* el, const String& tag)
{
  const XMLCh* par = el->getAttribute(tag.getWChars());
  if (*par == '\0') {
    return nullptr;
  }
  DString dpar = DString(par);
  return getNCRegion(&dpar, true);
}

/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Colorer Library.
 *
 * The Initial Developer of the Original Code is
 * Cail Lomecb <cail@nm.ru>.
 * Portions created by the Initial Developer are Copyright (C) 1999-2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Eugene Efremov <4mirror@mail.ru>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
