#include "colorer/parsers/HrcLibraryImpl.h"
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/NumberFormatException.hpp>
#include <xercesc/util/XMLDouble.hpp>
#include "colorer/base/XmlTagDefs.h"
#include "colorer/common/UStr.h"
#include "colorer/parsers/FileTypeImpl.h"
#include "colorer/xml/BaseEntityResolver.h"
#include "colorer/xml/XmlParserErrorHandler.h"

HrcLibrary::Impl::Impl()
{
  fileTypeHash.reserve(200);
  fileTypeVector.reserve(150);
  regionNamesHash.reserve(1000);
  regionNamesVector.reserve(1000);
  schemeHash.reserve(4000);
}

HrcLibrary::Impl::~Impl()
{
  for (const auto& it : fileTypeHash) {
    delete it.second;
  }

  for (const auto& it : schemeHash) {
    delete it.second;
  }

  for (auto it : regionNamesVector) {
    delete it;
  }

  for (const auto& it : schemeEntitiesHash) {
    delete it.second;
  }
}

void HrcLibrary::Impl::loadSource(XmlInputSource* is)
{
  if (!is) {
    throw HrcLibraryException("Can't open stream - 'null' is bad stream.");
  }

  XmlInputSource* istemp = current_input_source;
  current_input_source = is;
  try {
    parseHRC(*is);
  } catch (Exception&) {
    current_input_source = istemp;
    throw;
  }
  current_input_source = istemp;
}

void HrcLibrary::Impl::unloadFileType(FileType* filetype)
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

void HrcLibrary::Impl::loadFileType(FileType* filetype)
{
  auto* thisType = filetype;
  if (thisType == nullptr || filetype->pimpl->type_loading ||
      thisType->pimpl->input_source_loading || thisType->pimpl->load_broken)
  {
    return;
  }

  thisType->pimpl->input_source_loading = true;

  auto& input_source = thisType->pimpl->inputSource;
  try {
    loadSource(input_source.get());
  } catch (InputSourceException& e) {
    spdlog::error("Can't open source stream: {0}", e.what());
    thisType->pimpl->load_broken = true;
  } catch (HrcLibraryException& e) {
    spdlog::error("{0} [{1}]", e.what(),
                  thisType->pimpl->inputSource ? input_source->getPath() : "");
    thisType->pimpl->load_broken = true;
  } catch (Exception& e) {
    spdlog::error("{0} [{1}]", e.what(),
                  thisType->pimpl->inputSource ? input_source->getPath() : "");
    thisType->pimpl->load_broken = true;
  } catch (...) {
    spdlog::error("Unknown exception while loading {0}", input_source->getPath());
    thisType->pimpl->load_broken = true;
  }

  thisType->pimpl->input_source_loading = false;
}

FileType* HrcLibrary::Impl::chooseFileType(const UnicodeString* fileName,
                                           const UnicodeString* firstLine, int typeNo)
{
  FileType* best = nullptr;
  double max_prior = 0;
  const double DELTA = 1e-6;
  for (auto ret : fileTypeVector) {
    double prior = ret->pimpl->getPriority(fileName, firstLine);

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

FileType* HrcLibrary::Impl::getFileType(const UnicodeString* name)
{
  if (name == nullptr) {
    return nullptr;
  }
  auto filetype = fileTypeHash.find(*name);
  if (filetype != fileTypeHash.end())
    return filetype->second;
  else
    return nullptr;
}

FileType* HrcLibrary::Impl::enumerateFileTypes(unsigned int index)
{
  if (index < fileTypeVector.size()) {
    return fileTypeVector[index];
  }
  return nullptr;
}

size_t HrcLibrary::Impl::getFileTypesCount()
{
  return fileTypeVector.size();
}

size_t HrcLibrary::Impl::getRegionCount()
{
  return regionNamesVector.size();
}

const Region* HrcLibrary::Impl::getRegion(unsigned int id)
{
  if (id >= regionNamesVector.size()) {
    return nullptr;
  }
  return regionNamesVector[id];
}

const Region* HrcLibrary::Impl::getRegion(const UnicodeString* name)
{
  if (name == nullptr) {
    return nullptr;
  }
  return getNCRegion(name, false);  // regionNamesHash.get(name);
}

// protected methods

void HrcLibrary::Impl::parseHRC(const XmlInputSource& is)
{
  spdlog::debug("begin parse '{0}'", is.getPath());
  xercesc::XercesDOMParser xml_parser;
  XmlParserErrorHandler error_handler;
  BaseEntityResolver resolver;
  xml_parser.setErrorHandler(&error_handler);
  xml_parser.setXMLEntityResolver(&resolver);
  xml_parser.setLoadExternalDTD(false);
  xml_parser.setSkipDTDValidation(true);
  xml_parser.setDisableDefaultEntityResolution(true);
  xml_parser.parse(*is.getInputSource());
  if (error_handler.getSawErrors()) {
    throw HrcLibraryException("Error reading hrc file '" + is.getPath() + "'");
  }
  xercesc::DOMDocument* doc = xml_parser.getDocument();
  xercesc::DOMElement* root = doc->getDocumentElement();

  if (!root || !xercesc::XMLString::equals(root->getNodeName(), hrcTagHrc)) {
    throw HrcLibraryException(
        "Incorrect hrc-file structure. Main '<hrc>' block not found. Current file " + is.getPath());
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

  spdlog::debug("end parse '{0}'", is.getPath());
}

void HrcLibrary::Impl::parseHrcBlock(const xercesc::DOMElement* elem)
{
  for (auto node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      parseHrcBlockElements(node);
    }
    else if (node->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      for (auto sub_node = node->getFirstChild(); sub_node != nullptr;
           sub_node = sub_node->getNextSibling())
      {
        parseHrcBlockElements(sub_node);
      }
    }
  }
}

void HrcLibrary::Impl::parseHrcBlockElements(const xercesc::DOMNode* elem)
{
  if (elem->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
    if (auto sub_elem = dynamic_cast<const xercesc::DOMElement*>(elem)) {
      if (xercesc::XMLString::equals(elem->getNodeName(), hrcTagPrototype) ||
          xercesc::XMLString::equals(elem->getNodeName(), hrcTagPackage))
      {
        addPrototype(sub_elem);
      }
      else if (xercesc::XMLString::equals(elem->getNodeName(), hrcTagType)) {
        addType(sub_elem);
      }
      else if (xercesc::XMLString::equals(elem->getNodeName(), hrcTagAnnotation)) {
        // not read annotation
      }
      else {
        spdlog::warn("Unused element '{0}'. Current file {1}.",
                     UStr::to_stdstr(elem->getNodeName()), current_input_source->getPath());
      }
    }
  }
}

void HrcLibrary::Impl::addPrototype(const xercesc::DOMElement* elem)
{
  const auto typeName = elem->getAttribute(hrcPrototypeAttrName);
  const auto typeGroup = elem->getAttribute(hrcPrototypeAttrGroup);
  const auto typeDescription = elem->getAttribute(hrcPrototypeAttrDescription);
  if (UStr::isEmpty(typeName)) {
    spdlog::error("Found unnamed prototype/package. Skipped.");
    return;
  }

  UnicodeString tname = UnicodeString(typeName);
  auto ft = fileTypeHash.find(tname);
  if (ft != fileTypeHash.end()) {
    unloadFileType(ft->second);
    spdlog::warn("Duplicate prototype '{0}'. First version unloaded, current is loading.", tname);
  }

  auto* type = new FileType(tname, UStr::isEmpty(typeGroup) ? tname : typeGroup,
                            UStr::isEmpty(typeDescription) ? tname : typeDescription);
  auto& ptype = type->pimpl;

  if (xercesc::XMLString::equals(elem->getNodeName(), hrcTagPackage)) {
    ptype->isPackage = true;
  }

  parsePrototypeBlock(elem, type);

  fileTypeHash.emplace(tname, type);
  if (!ptype->isPackage) {
    fileTypeVector.push_back(type);
  }
}

void HrcLibrary::Impl::parsePrototypeBlock(const xercesc::DOMElement* elem,
                                           FileType* current_parse_prototype)
{
  for (auto node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      if (auto* sub_elem = dynamic_cast<xercesc::DOMElement*>(node)) {
        if (xercesc::XMLString::equals(sub_elem->getNodeName(), hrcTagLocation)) {
          addPrototypeLocation(sub_elem, current_parse_prototype);
        }
        else if (!current_parse_prototype->pimpl->isPackage &&
                 (xercesc::XMLString::equals(sub_elem->getNodeName(), hrcTagFilename) ||
                  xercesc::XMLString::equals(sub_elem->getNodeName(), hrcTagFirstline)))
        {
          addPrototypeDetectParam(sub_elem, current_parse_prototype);
        }
        else if (!current_parse_prototype->pimpl->isPackage &&
                 (xercesc::XMLString::equals(sub_elem->getNodeName(), hrcTagParametrs)))
        {
          addPrototypeParameters(sub_elem, current_parse_prototype);
        }
        else if (xercesc::XMLString::equals(elem->getNodeName(), hrcTagAnnotation)) {
          // not read annotation
        }
        else {
          spdlog::warn("Unused element '{0}' in prototype '{1}'. Current file {2}.",
                       UStr::to_stdstr(elem->getNodeName()), current_parse_prototype->pimpl->name,
                       current_input_source->getPath());
        }
      }
    }
  }
}

void HrcLibrary::Impl::addPrototypeLocation(const xercesc::DOMElement* elem,
                                            FileType* current_parse_prototype)
{
  const XMLCh* locationLink = elem->getAttribute(hrcLocationAttrLink);
  if (UStr::isEmpty(locationLink)) {
    spdlog::error("Bad 'location' link attribute in prototype '{0}'",
                  current_parse_prototype->pimpl->name);
    return;
  }
  current_parse_prototype->pimpl->inputSource = current_input_source->createRelative(locationLink);
}

void HrcLibrary::Impl::addPrototypeDetectParam(const xercesc::DOMElement* elem,
                                               FileType* current_parse_prototype)
{
  if (elem->getFirstChild() == nullptr ||
      elem->getFirstChild()->getNodeType() != xercesc::DOMNode::TEXT_NODE)
  {
    spdlog::warn("Bad '{0}' element in prototype '{1}'", UStr::to_stdstr(elem->getNodeName()),
                 current_parse_prototype->pimpl->name);
    return;
  }
  auto elem_text = dynamic_cast<xercesc::DOMText*>(elem->getFirstChild());
  if (!elem_text) {
    spdlog::error("Fault read value of {0} in {1}", UStr::to_stdstr(elem->getNodeName()),
                  current_parse_prototype->pimpl->name);
    return;
  }

  UnicodeString dmatch = UnicodeString(elem_text->getData());
  auto matchRE = std::make_unique<CRegExp>(&dmatch);
  matchRE->setPositionMoves(true);
  if (!matchRE->isOk()) {
    spdlog::warn("Fault compiling chooser RE '{0}' in prototype '{1}'", dmatch,
                 current_parse_prototype->pimpl->name);
    return;
  }
  auto ctype = xercesc::XMLString::equals(elem->getNodeName(), hrcTagFilename)
      ? FileTypeChooser::ChooserType::CT_FILENAME
      : FileTypeChooser::ChooserType::CT_FIRSTLINE;
  double prior = ctype == FileTypeChooser::ChooserType::CT_FILENAME ? 2 : 1;
  auto weight = elem->getAttribute(hrcFilenameAttrWeight);
  if (!UStr::isEmpty(weight)) {
    try {
      auto w = xercesc::XMLDouble(weight);
      if (w.getValue() < 0) {
        spdlog::warn(
            "Weight must be greater than 0. Current value {0}. Default value will be used. Current "
            "file {1}.",
            w.getValue(), current_input_source->getPath());
      }
      else {
        prior = w.getValue();
      }
    } catch (xercesc::NumberFormatException& toCatch) {
      spdlog::warn(
          "Weight '{0}' is not valid for the prototype '{1}'. Message: {2}. Default value will be "
          "used. Current file {3}.",
          UStr::to_stdstr(weight), current_parse_prototype->getName(),
          UStr::to_stdstr(toCatch.getMessage()), current_input_source->getPath());
    }
  }
  current_parse_prototype->pimpl->chooserVector.emplace_back(ctype, prior, matchRE.release());
}

void HrcLibrary::Impl::addPrototypeParameters(const xercesc::DOMNode* elem,
                                              FileType* current_parse_prototype)
{
  for (auto node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      if (auto* subelem = dynamic_cast<xercesc::DOMElement*>(node)) {
        if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagParam)) {
          auto name = subelem->getAttribute(hrcParamAttrName);
          auto value = subelem->getAttribute(hrcParamAttrValue);
          auto descr = subelem->getAttribute(hrcParamAttrDescription);
          if (UStr::isEmpty(name) || UStr::isEmpty(value)) {
            spdlog::warn("Bad parameter in prototype '{0}'", current_parse_prototype->getName());
            continue;
          }
          auto& tp =
              current_parse_prototype->pimpl->addParam(UnicodeString(name), UnicodeString(value));
          if (!UStr::isEmpty(descr)) {
            tp.description.emplace(descr);
          }
        }
        else {
          spdlog::warn("Unused element '{0}' in prototype '{1}'. Current file {2}.",
                       UStr::to_stdstr(elem->getNodeName()), current_parse_prototype->pimpl->name,
                       current_input_source->getPath());
        }
      }
    }
    if (node->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      addPrototypeParameters(node, current_parse_prototype);
    }
  }
}

void HrcLibrary::Impl::addType(const xercesc::DOMElement* elem)
{
  auto typeName = elem->getAttribute(hrcTypeAttrName);

  if (UStr::isEmpty(typeName)) {
    spdlog::error("Unnamed type found");
    return;
  }
  auto d_name = UnicodeString(typeName);
  auto type_ref = fileTypeHash.find(d_name);
  if (type_ref == fileTypeHash.end()) {
    spdlog::error("Type '%s' without prototype", d_name);
    return;
  }
  auto type = type_ref->second;
  if (type->pimpl->type_loading) {
    spdlog::warn("Type '{0}' is loading already. Current file {1}", UStr::to_stdstr(typeName),
                 current_input_source->getPath());
    return;
  }

  type->pimpl->type_loading = true;

  FileType* o_parseType = current_parse_type;
  current_parse_type = type;

  parseTypeBlock(elem);

  auto baseSchemeName = qualifyOwnName(type->getName());
  if (baseSchemeName != nullptr) {
    auto sh = schemeHash.find(*baseSchemeName);
    type->pimpl->baseScheme = sh == schemeHash.end() ? nullptr : sh->second;
  }
  if (type->pimpl->baseScheme == nullptr && !type->pimpl->isPackage) {
    spdlog::warn("Type '{0}' has no default scheme", UStr::to_stdstr(typeName));
  }
  type->pimpl->loadDone = true;
  current_parse_type = o_parseType;
}

void HrcLibrary::Impl::parseTypeBlock(const xercesc::DOMNode* elem)
{
  for (auto node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      if (auto* subelem = dynamic_cast<xercesc::DOMElement*>(node)) {
        if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagRegion)) {
          addTypeRegion(subelem);
        }
        else if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagEntity)) {
          addTypeEntity(subelem);
        }
        else if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagImport)) {
          addTypeImport(subelem);
        }
        else if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagScheme)) {
          addScheme(subelem);
        }
        else if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagAnnotation)) {
          // not read anotation
        }
      }
    }
    // случай entity ссылки на другой файл
    if (node->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      parseTypeBlock(node);
    }
  }
}

void HrcLibrary::Impl::addTypeRegion(const xercesc::DOMElement* elem)
{
  auto regionName = elem->getAttribute(hrcRegionAttrName);
  auto regionParent = elem->getAttribute(hrcRegionAttrParent);
  auto regionDescr = elem->getAttribute(hrcRegionAttrDescription);
  if (UStr::isEmpty(regionName)) {
    spdlog::error("No 'name' attribute in <region> element");
    return;
  }
  auto qname1 = qualifyOwnName(UnicodeString(regionName));
  if (qname1 == nullptr) {
    return;
  }
  if (regionNamesHash.find(*qname1) != regionNamesHash.end()) {
    spdlog::warn("Duplicate region '{0}' definition in type '{1}'", *qname1,
                 current_parse_type->getName());
    return;
  }

  UnicodeString d_regionparent = UnicodeString(regionParent);
  UnicodeString* qname2 = qualifyForeignName(*regionParent != '\0' ? &d_regionparent : nullptr,
                                             QualifyNameType::QNT_DEFINE, true);

  UnicodeString regiondescr = UnicodeString(regionDescr);
  const Region* region =
      new Region(*qname1, &regiondescr, getRegion(qname2), regionNamesVector.size());
  regionNamesVector.push_back(region);
  regionNamesHash.emplace(*qname1, region);

  delete qname2;
}

void HrcLibrary::Impl::addTypeEntity(const xercesc::DOMElement* elem)
{
  auto entityName = elem->getAttribute(hrcEntityAttrName);
  auto entityValue = elem->getAttribute(hrcEntityAttrValue);
  if (UStr::isEmpty(entityName) || entityValue == nullptr) {
    spdlog::error("Bad entity attributes");
    return;
  }
  auto dentityValue = UnicodeString(entityValue);
  auto qname1 = qualifyOwnName(UnicodeString(entityName));
  uUnicodeString qname2 = useEntities(&dentityValue);
  if (qname1 != nullptr && qname2 != nullptr) {
    schemeEntitiesHash.emplace(*qname1, qname2.release());
  }
}

void HrcLibrary::Impl::addTypeImport(const xercesc::DOMElement* elem)
{
  auto typeParam = elem->getAttribute(hrcImportAttrType);
  UnicodeString typeparam = UnicodeString(typeParam);
  if (UStr::isEmpty(typeParam) || fileTypeHash.find(typeparam) == fileTypeHash.end()) {
    spdlog::error("Import with bad '{0}' attribute in type '{1}'", typeparam,
                  current_parse_type->pimpl->name);
    return;
  }
  current_parse_type->pimpl->importVector.emplace_back(typeParam);
}

void HrcLibrary::Impl::addScheme(const xercesc::DOMElement* elem)
{
  auto schemeName = elem->getAttribute(hrcSchemeAttrName);
  UnicodeString dschemeName = UnicodeString(schemeName);
  // todo check schemeName
  auto qSchemeName = qualifyOwnName(dschemeName);
  if (qSchemeName == nullptr) {
    spdlog::error("bad scheme name in type '{0}'", current_parse_type->pimpl->name);
    return;
  }
  if (schemeHash.find(*qSchemeName) != schemeHash.end() ||
      disabledSchemes.find(*qSchemeName) != disabledSchemes.end())
  {
    spdlog::error("duplicate scheme name '{0}'", *qSchemeName);
    return;
  }

  auto* scheme = new SchemeImpl(qSchemeName.get());
  scheme->fileType = current_parse_type;

  schemeHash.emplace(*scheme->getName(), scheme);
  const XMLCh* condIf = elem->getAttribute(hrcSchemeAttrIf);
  const XMLCh* condUnless = elem->getAttribute(hrcSchemeAttrUnless);
  const UnicodeString* p1 = current_parse_type->getParamValue(UnicodeString(condIf));
  const UnicodeString* p2 = current_parse_type->getParamValue(UnicodeString(condUnless));
  if ((*condIf != '\0' && p1 && p1->compare("true") != 0) ||
      (*condUnless != '\0' && p2 && p2->compare("true") == 0))
  {
    // disabledSchemes.put(scheme->schemeName, 1);
    return;
  }
  parseSchemeBlock(scheme, elem);
}

void HrcLibrary::Impl::parseSchemeBlock(SchemeImpl* scheme, const xercesc::DOMNode* elem)
{
  for (xercesc::DOMNode* node = elem->getFirstChild(); node != nullptr;
       node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      auto* subelem = dynamic_cast<xercesc::DOMElement*>(node);
      if (subelem) {
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
    }
    if (node->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      parseSchemeBlock(scheme, node);
    }
  }
}

void HrcLibrary::Impl::addSchemeInherit(SchemeImpl* scheme, const xercesc::DOMElement* elem)
{
  const XMLCh* nqSchemeName = elem->getAttribute(hrcInheritAttrScheme);
  if (*nqSchemeName == '\0') {
    spdlog::error("empty scheme name in inheritance operator in scheme '{0}'",
                  *scheme->schemeName.get());
    return;
  }
  auto scheme_node = std::make_unique<SchemeNode>();
  scheme_node->type = SchemeNode::SchemeNodeType::SNT_INHERIT;
  scheme_node->schemeName = std::make_unique<UnicodeString>(nqSchemeName);
  UnicodeString dnqSchemeName = UnicodeString(nqSchemeName);
  UnicodeString* schemeName =
      qualifyForeignName(&dnqSchemeName, QualifyNameType::QNT_SCHEME, false);
  if (schemeName == nullptr) {
    //        if (errorHandler != null) errorHandler->warning(StringBuffer("forward inheritance of
    //        '")+nqSchemeName+"'. possible inherit loop with
    //        '"+scheme->schemeName+"'"); delete next; continue;
  }
  else {
    scheme_node->scheme = schemeHash.find(*schemeName)->second;
  }
  if (schemeName != nullptr) {
    scheme_node->schemeName.reset(schemeName);
  }

  for (xercesc::DOMNode* node = elem->getFirstChild(); node != nullptr;
       node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      auto* subelem = dynamic_cast<xercesc::DOMElement*>(node);
      if (subelem && xercesc::XMLString::equals(subelem->getNodeName(), hrcTagVirtual)) {
        const XMLCh* x_schemeName = subelem->getAttribute(hrcVirtualAttrScheme);
        const XMLCh* x_substName = subelem->getAttribute(hrcVirtualAttrSubstScheme);
        if (*x_schemeName == '\0' || *x_substName == '\0') {
          spdlog::error("bad virtualize attributes in scheme '{0}'", *scheme->schemeName.get());
          continue;
        }
        UnicodeString d_schemeName = UnicodeString(x_schemeName);
        UnicodeString d_substName = UnicodeString(x_substName);
        scheme_node->virtualEntryVector.push_back(new VirtualEntry(&d_schemeName, &d_substName));
      }
    }
  }
  scheme->nodes.push_back(std::move(scheme_node));
}

void HrcLibrary::Impl::addSchemeRegexp(SchemeImpl* scheme, const xercesc::DOMElement* elem)
{
  const XMLCh* matchParam = elem->getAttribute(hrcRegexpAttrMatch);
  if (*matchParam == '\0') {
    for (xercesc::DOMNode* child = elem->getFirstChild(); child != nullptr;
         child = child->getNextSibling())
    {
      if (child->getNodeType() == xercesc::DOMNode::CDATA_SECTION_NODE) {
        matchParam = ((xercesc::DOMCDATASection*) child)->getData();
        break;
      }
      if (child->getNodeType() == xercesc::DOMNode::TEXT_NODE) {
        const XMLCh* matchParam1;
        matchParam1 = ((xercesc::DOMText*) child)->getData();
        xercesc::XMLString::trim((XMLCh*) matchParam1);
        if (*matchParam1 != '\0') {
          matchParam = matchParam1;
          break;
        }
      }
    }
  }
  if (matchParam == nullptr) {
    spdlog::error("no 'match' in regexp in scheme '{0}'", *scheme->schemeName.get());
    return;
  }
  UnicodeString dmatchParam = UnicodeString(matchParam);
  uUnicodeString entMatchParam = useEntities(&dmatchParam);
  auto scheme_node = std::make_unique<SchemeNode>();
  UnicodeString dhrcRegexpAttrPriority = UnicodeString(elem->getAttribute(hrcRegexpAttrPriority));
  scheme_node->lowPriority = UnicodeString("low").compare(dhrcRegexpAttrPriority) == 0;
  scheme_node->type = SchemeNode::SchemeNodeType::SNT_RE;
  scheme_node->start = std::make_unique<CRegExp>(entMatchParam.get());
  if (!scheme_node->start || !scheme_node->start->isOk()) {
    spdlog::error("fault compiling regexp '{0}' in scheme '{1}'", *entMatchParam,
                  *scheme->schemeName.get());
    return;
  }
  scheme_node->start->setPositionMoves(false);
  scheme_node->end = nullptr;

  loadRegions(scheme_node.get(), elem, true);
  if (scheme_node->region) {
    scheme_node->regions[0] = scheme_node->region;
  }

  scheme->nodes.push_back(std::move(scheme_node));
}

void HrcLibrary::Impl::addSchemeBlock(SchemeImpl* scheme, const xercesc::DOMElement* elem)
{
  const XMLCh* sParam = elem->getAttribute(hrcBlockAttrStart);
  const XMLCh* eParam = elem->getAttribute(hrcBlockAttrEnd);

  xercesc::DOMElement *eStart = nullptr, *eEnd = nullptr;

  for (xercesc::DOMNode* blkn = elem->getFirstChild();
       blkn && !(*eParam != '\0' && *sParam != '\0'); blkn = blkn->getNextSibling())
  {
    xercesc::DOMElement* blkel;
    if (blkn->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      blkel = dynamic_cast<xercesc::DOMElement*>(blkn);
      if (!blkel)
        continue;
    }
    else {
      continue;
    }

    const XMLCh* p = nullptr;
    if (blkel->getAttributeNode(hrcBlockAttrMatch)) {
      p = blkel->getAttribute(hrcBlockAttrMatch);
    }
    else {
      for (xercesc::DOMNode* child = blkel->getFirstChild(); child != nullptr;
           child = child->getNextSibling())
      {
        if (child->getNodeType() == xercesc::DOMNode::CDATA_SECTION_NODE) {
          p = ((xercesc::DOMCDATASection*) child)->getData();
          break;
        }
        if (child->getNodeType() == xercesc::DOMNode::TEXT_NODE) {
          const XMLCh* p1;
          p1 = ((xercesc::DOMText*) child)->getData();
          xercesc::XMLString::trim((XMLCh*) p1);
          if (*p1 != '\0') {
            p = p1;
            break;
          }
        }
      }
    }

    if (xercesc::XMLString::equals(blkel->getNodeName(), hrcBlockAttrStart)) {
      sParam = p;
      eStart = blkel;
    }
    if (xercesc::XMLString::equals(blkel->getNodeName(), hrcBlockAttrEnd)) {
      eParam = p;
      eEnd = blkel;
    }
  }

  uUnicodeString startParam;
  uUnicodeString endParam;
  UnicodeString dsParam = UnicodeString(sParam);
  if (!(startParam = useEntities(&dsParam))) {
    spdlog::error("'start' block attribute not found in scheme '{0}'", *scheme->schemeName.get());
    return;
  }
  UnicodeString deParam = UnicodeString(eParam);
  if (!(endParam = useEntities(&deParam))) {
    spdlog::error("'end' block attribute not found in scheme '{0}'", *scheme->schemeName.get());
    return;
  }
  const XMLCh* schemeName = elem->getAttribute(hrcBlockAttrScheme);
  if (*schemeName == '\0') {
    spdlog::error("block with bad scheme attribute in scheme '{0}'", *scheme->schemeName.get());
    return;
  }
  auto scheme_node = std::make_unique<SchemeNode>();
  scheme_node->schemeName = std::make_unique<UnicodeString>(schemeName);
  UnicodeString attr_pr = UnicodeString(elem->getAttribute(hrcBlockAttrPriority));
  UnicodeString attr_cpr = UnicodeString(elem->getAttribute(hrcBlockAttrContentPriority));
  UnicodeString attr_ireg = UnicodeString(elem->getAttribute(hrcBlockAttrInnerRegion));
  scheme_node->lowPriority = UnicodeString("low").compare(attr_pr) == 0;
  scheme_node->lowContentPriority = UnicodeString("low").compare(attr_cpr) == 0;
  scheme_node->innerRegion = UnicodeString("yes").compare(attr_ireg) == 0;
  scheme_node->type = SchemeNode::SchemeNodeType::SNT_SCHEME;
  scheme_node->start = std::make_unique<CRegExp>(startParam.get());
  scheme_node->start->setPositionMoves(false);
  if (!scheme_node->start->isOk()) {
    spdlog::error("fault compiling regexp '{0}' in scheme '{1}'", *startParam.get(),
                  *scheme->schemeName.get());
  }
  scheme_node->end = std::make_unique<CRegExp>();
  scheme_node->end->setPositionMoves(true);
  scheme_node->end->setBackRE(scheme_node->start.get());
  scheme_node->end->setRE(endParam.get());
  if (!scheme_node->end->isOk()) {
    spdlog::error("fault compiling regexp '{0}' in scheme '{1}'", *endParam.get(),
                  *scheme->schemeName.get());
  }

  // !! EE
  loadBlockRegions(scheme_node.get(), elem);
  loadRegions(scheme_node.get(), eStart, true);
  loadRegions(scheme_node.get(), eEnd, false);
  scheme->nodes.push_back(std::move(scheme_node));
}

void HrcLibrary::Impl::addSchemeKeywords(SchemeImpl* scheme, const xercesc::DOMElement* elem)
{
  char16_t rg_tmpl[] = u"region\0";
  const Region* brgn = getNCRegion(elem, rg_tmpl);
  if (brgn == nullptr) {
    return;
  }

  auto scheme_node = std::make_unique<SchemeNode>();
  UnicodeString dhrcKeywordsAttrIgnorecase =
      UnicodeString(elem->getAttribute(hrcKeywordsAttrIgnorecase));
  UnicodeString dhrcKeywordsAttrPriority =
      UnicodeString(elem->getAttribute(hrcKeywordsAttrPriority));
  bool isCase = UnicodeString("yes").compare(dhrcKeywordsAttrIgnorecase) != 0;
  scheme_node->lowPriority = UnicodeString("normal").compare(dhrcKeywordsAttrPriority) != 0;

  const XMLCh* worddiv = elem->getAttribute(hrcKeywordsAttrWorddiv);

  scheme_node->worddiv = nullptr;
  if (*worddiv != '\0') {
    UnicodeString dworddiv = UnicodeString(worddiv);
    uUnicodeString entWordDiv = useEntities(&dworddiv);
    scheme_node->worddiv = UStr::createCharClass(*entWordDiv.get(), 0, nullptr, false);
    if (scheme_node->worddiv == nullptr) {
      spdlog::error("fault compiling worddiv regexp '{0}' in scheme '{1}'", *entWordDiv,
                    *scheme->schemeName.get());
    }
  }

  scheme_node->kwList = std::make_unique<KeywordList>();
  auto count = getSchemeKeywordsCount(elem);

  scheme_node->kwList->kwList = new KeywordInfo[count];
  scheme_node->kwList->num = 0;
  scheme_node->kwList->matchCase = isCase;
  scheme_node->type = SchemeNode::SchemeNodeType::SNT_KEYWORDS;

  for (xercesc::DOMNode* keywrd = elem->getFirstChild(); keywrd; keywrd = keywrd->getNextSibling())
  {
    if (keywrd->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      addKeyword(scheme_node.get(), brgn, dynamic_cast<xercesc::DOMElement*>(keywrd));
      continue;
    }
    if (keywrd->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      for (xercesc::DOMNode* keywrd2 = keywrd->getFirstChild(); keywrd2;
           keywrd2 = keywrd2->getNextSibling())
      {
        if (keywrd2->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
          addKeyword(scheme_node.get(), brgn, dynamic_cast<xercesc::DOMElement*>(keywrd2));
        }
      }
    }
  }
  scheme_node->kwList->sortList();
  scheme_node->kwList->substrIndex();
  scheme_node->kwList->firstChar->freeze();
  scheme->nodes.push_back(std::move(scheme_node));
}

void HrcLibrary::Impl::addKeyword(SchemeNode* scheme_node, const Region* brgn,
                                  const xercesc::DOMElement* elem)
{
  int type = 0;
  if (xercesc::XMLString::equals(elem->getNodeName(), hrcTagWord)) {
    type = 1;
  }
  if (xercesc::XMLString::equals(elem->getNodeName(), hrcTagSymb)) {
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
    rgn = getNCRegion(elem, hrcWordAttrRegion);
  }

  int pos = scheme_node->kwList->num;
  scheme_node->kwList->kwList[pos].keyword = std::make_unique<UnicodeString>(UnicodeString(param));
  scheme_node->kwList->kwList[pos].region = rgn;
  scheme_node->kwList->kwList[pos].isSymbol = (type == 2);
  scheme_node->kwList->kwList[pos].ssShorter = -1;
  auto first_char = scheme_node->kwList->firstChar.get();
  first_char->add(param[0]);
  if (!scheme_node->kwList->matchCase) {
    first_char->add(u_toupper(param[0]));
    first_char->add(u_tolower(param[0]));
    first_char->add(u_totitle(param[0]));
  }
  scheme_node->kwList->num++;
  if (scheme_node->kwList->minKeywordLength > scheme_node->kwList->kwList[pos].keyword->length()) {
    scheme_node->kwList->minKeywordLength = scheme_node->kwList->kwList[pos].keyword->length();
  }
}

int HrcLibrary::Impl::getSchemeKeywordsCount(const xercesc::DOMNode* elem)
{
  int result = 0;
  for (xercesc::DOMNode* keywrd_count = elem->getFirstChild(); keywrd_count;
       keywrd_count = keywrd_count->getNextSibling())
  {
    if (keywrd_count->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      if (xercesc::XMLString::equals(keywrd_count->getNodeName(), hrcTagWord) ||
          xercesc::XMLString::equals(keywrd_count->getNodeName(), hrcTagSymb))
      {
        result++;
      }
      continue;
    }
    if (keywrd_count->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      result += getSchemeKeywordsCount(keywrd_count);
    }
  }
  return result;
}

void HrcLibrary::Impl::loadRegions(SchemeNode* node, const xercesc::DOMElement* el, bool st)
{
  char16_t rg_tmpl[] = u"region\0\0";

  if (el) {
    if (node->region == nullptr) {
      node->region = getNCRegion(el, rg_tmpl);
    }

    for (int i = 0; i < REGIONS_NUM; i++) {
      rg_tmpl[6] = static_cast<XMLCh>((i < 0xA ? i : i + 39) + '0');

      if (st) {
        node->regions[i] = getNCRegion(el, rg_tmpl);
      }
      else {
        node->regione[i] = getNCRegion(el, rg_tmpl);
      }
    }
  }

  for (int i = 0; i < NAMED_REGIONS_NUM; i++) {
    if (st) {
      node->regionsn[i] = getNCRegion(node->start->getBracketName(i), false);
    }
    else {
      node->regionen[i] = getNCRegion(node->end->getBracketName(i), false);
    }
  }
}

void HrcLibrary::Impl::loadBlockRegions(SchemeNode* node, const xercesc::DOMElement* el)
{
  int i;
  char16_t rg_tmpl[] = u"region\0\0\0";

  node->region = getNCRegion(el, rg_tmpl);
  for (i = 0; i < REGIONS_NUM; i++) {
    rg_tmpl[6] = '0';
    rg_tmpl[7] = static_cast<XMLCh>((i < 0xA ? i : i + 39) + '0');
    node->regions[i] = getNCRegion(el, rg_tmpl);
    rg_tmpl[6] = '1';
    node->regione[i] = getNCRegion(el, rg_tmpl);
  }
}

void HrcLibrary::Impl::updateLinks()
{
  while (structureChanged) {
    structureChanged = false;
    for (auto& scheme_it : schemeHash) {
      SchemeImpl* scheme = scheme_it.second;
      if (!scheme->fileType->pimpl->loadDone) {
        continue;
      }
      FileType* old_parseType = current_parse_type;
      current_parse_type = scheme->fileType;
      for (size_t sni = 0; sni < scheme->nodes.size(); sni++) {
        auto& snode = scheme->nodes[sni];
        if (snode->schemeName != nullptr &&
            (snode->type == SchemeNode::SchemeNodeType::SNT_SCHEME ||
             snode->type == SchemeNode::SchemeNodeType::SNT_INHERIT) &&
            snode->scheme == nullptr)
        {
          UnicodeString* schemeName =
              qualifyForeignName(snode->schemeName.get(), QualifyNameType::QNT_SCHEME, true);
          if (schemeName != nullptr) {
            snode->scheme = schemeHash.find(*schemeName)->second;
          }
          else {
            spdlog::error("cannot resolve scheme name '{0}' in scheme '{1}'", *snode->schemeName,
                          *scheme->schemeName.get());
          }
          delete schemeName;
          snode->schemeName.reset();
        }
        if (snode->type == SchemeNode::SchemeNodeType::SNT_INHERIT) {
          for (auto vt : snode->virtualEntryVector) {
            if (vt->virtScheme == nullptr && vt->virtSchemeName != nullptr) {
              UnicodeString* vsn =
                  qualifyForeignName(vt->virtSchemeName.get(), QualifyNameType::QNT_SCHEME, true);
              if (vsn) {
                vt->virtScheme = schemeHash.find(*vsn)->second;
              }
              else {
                spdlog::error("cannot virtualize scheme '{0}' in scheme '{1}'",
                              *vt->virtSchemeName.get(), *scheme->schemeName.get());
              }
              delete vsn;
              vt->virtSchemeName.reset();
            }
            if (vt->substScheme == nullptr && vt->substSchemeName != nullptr) {
              UnicodeString* vsn =
                  qualifyForeignName(vt->substSchemeName.get(), QualifyNameType::QNT_SCHEME, true);
              if (vsn) {
                vt->substScheme = schemeHash.find(*vsn)->second;
              }
              else {
                spdlog::error("cannot virtualize using subst-scheme scheme '{0}' in scheme '{1}'",
                              *vt->substSchemeName.get(), *scheme->schemeName.get());
              }
              delete vsn;
              vt->substSchemeName.reset();
            }
          }
        }
      }
      current_parse_type = old_parseType;
      if (structureChanged) {
        break;
      }
    }
  }
}

uUnicodeString HrcLibrary::Impl::qualifyOwnName(const UnicodeString& name)
{
  auto colon = name.indexOf(':');
  if (colon != -1) {
    if (UnicodeString(name, 0, colon).compare(current_parse_type->pimpl->name) != 0) {
      spdlog::error("type name qualifer in '{0}' doesn't match current type '{1}'", name,
                    current_parse_type->pimpl->name);
      return nullptr;
    }
    else {
      return std::make_unique<UnicodeString>(name);
    }
  }
  else {
    auto sbuf = std::make_unique<UnicodeString>(current_parse_type->getName());
    sbuf->append(":").append(name);
    return sbuf;
  }
}

bool HrcLibrary::Impl::checkNameExist(const UnicodeString* name, FileType* parseType,
                                      QualifyNameType qntype, bool logErrors)
{
  if (qntype == QualifyNameType::QNT_DEFINE && regionNamesHash.find(*name) == regionNamesHash.end())
  {
    if (logErrors)
      spdlog::error("region '{0}', referenced in type '{1}', is not defined", *name,
                    parseType->pimpl->name);
    return false;
  }
  else if (qntype == QualifyNameType::QNT_ENTITY &&
           schemeEntitiesHash.find(*name) == schemeEntitiesHash.end())
  {
    if (logErrors)
      spdlog::error("entity '{0}', referenced in type '{1}', is not defined", *name,
                    parseType->pimpl->name);
    return false;
  }
  else if (qntype == QualifyNameType::QNT_SCHEME && schemeHash.find(*name) == schemeHash.end()) {
    if (logErrors)
      spdlog::error("scheme '{0}', referenced in type '{1}', is not defined", *name,
                    parseType->pimpl->name);
    return false;
  }
  return true;
}

UnicodeString* HrcLibrary::Impl::qualifyForeignName(const UnicodeString* name,
                                                    QualifyNameType qntype, bool logErrors)
{
  if (name == nullptr) {
    return nullptr;
  }
  auto colon = name->indexOf(':');
  if (colon != -1) {  // qualified name
    UnicodeString prefix(*name, 0, colon);
    auto ft = fileTypeHash.find(prefix);
    FileType* prefType = nullptr;
    if (ft != fileTypeHash.end()) {
      prefType = ft->second;
    }

    if (prefType == nullptr) {
      if (logErrors) {
        spdlog::error("type name qualifer in '{0}' doesn't match any type", *name);
      }
      return nullptr;
    }
    else if (!prefType->pimpl->type_loading) {
      loadFileType(prefType);
    }
    if (prefType == current_parse_type || prefType->pimpl->type_loading) {
      return checkNameExist(name, prefType, qntype, logErrors) ? (new UnicodeString(*name))
                                                               : nullptr;
    }
  }
  else {  // unqualified name
    for (int idx = -1; current_parse_type != nullptr &&
         idx < static_cast<int>(current_parse_type->pimpl->importVector.size());
         idx++)
    {
      auto tname = current_parse_type->getName();
      if (idx > -1) {
        tname = current_parse_type->pimpl->importVector.at(idx);
      }
      FileType* importer = fileTypeHash.find(tname)->second;
      if (!importer->pimpl->type_loading) {
        loadFileType(importer);
      }

      auto qname = std::make_unique<UnicodeString>(tname);
      qname->append(":").append(*name);
      if (checkNameExist(qname.get(), importer, qntype, false)) {
        return qname.release();
      }
    }
    if (logErrors) {
      spdlog::error("unqualified name '{0}' doesn't belong to any imported type [{1}]", *name,
                    current_input_source->getPath());
    }
  }
  return nullptr;
}

uUnicodeString HrcLibrary::Impl::useEntities(const UnicodeString* name)
{
  int copypos = 0;
  int32_t epos = 0;

  if (!name) {
    return nullptr;
  }
  auto newname = std::make_unique<UnicodeString>();

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
    auto elpos = name->indexOf(';', epos);
    if (elpos == -1) {
      epos = name->length();
      break;
    }
    UnicodeString enname(*name, epos + 1, elpos - epos - 1);

    UnicodeString* qEnName = qualifyForeignName(&enname, QualifyNameType::QNT_ENTITY, true);
    const UnicodeString* enval = nullptr;
    if (qEnName != nullptr) {
      enval = schemeEntitiesHash.find(*qEnName)->second;
      delete qEnName;
    }
    if (enval == nullptr) {
      epos++;
      continue;
    }
    newname->append(UnicodeString(*name, copypos, epos - copypos));
    newname->append(*enval);
    epos = elpos + 1;
    copypos = epos;
  }
  if (epos > copypos) {
    newname->append(UnicodeString(*name, copypos, epos - copypos));
  }
  return newname;
}

const Region* HrcLibrary::Impl::getNCRegion(const UnicodeString* name, bool logErrors)
{
  if (name == nullptr || name->isEmpty()) {
    return nullptr;
  }
  const Region* reg;
  UnicodeString* qname = qualifyForeignName(name, QualifyNameType::QNT_DEFINE, logErrors);
  if (qname == nullptr) {
    return nullptr;
  }
  auto reg_ = regionNamesHash.find(*qname);
  if (reg_ != regionNamesHash.end()) {
    reg = reg_->second;
  }
  else {
    reg = nullptr;
  }

  delete qname;
  /** Check for 'default' region request.
      Regions with this name are always transparent
  */
  if (reg != nullptr) {
    auto s_name = reg->getName();
    auto idx = s_name.indexOf(":default");
    if (idx != -1 && idx + 8 == s_name.length()) {
      return nullptr;
    }
  }
  return reg;
}

const Region* HrcLibrary::Impl::getNCRegion(const xercesc::DOMElement* el, const XMLCh* tag)
{
  const XMLCh* par = el->getAttribute(tag);
  if (*par == '\0') {
    return nullptr;
  }
  UnicodeString dpar = UnicodeString(par);
  return getNCRegion(&dpar, true);
}
