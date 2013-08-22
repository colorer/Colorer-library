
#include<stdio.h>
#include<colorer/parsers/helpers/HRCParserHelpers.h>
#include<colorer/parsers/HRCParserImpl.h>
#include <xml/XmlParserErrorHandler.h>
#include <xml/XmlInputSource.h>
#include <xml/BaseEntityResolver.h>
#include <xml/XmlTagDefs.h>

#define CURRENT_FILE SString(" Current file ")+ DString(curInputSource->getInputSource()->getSystemId()) +DString(".")

HRCParserImpl::HRCParserImpl()
 : fileTypeHash(200), fileTypeVector(150), schemeHash(4000),
 regionNamesVector(1000, 200), regionNamesHash(1000)
{
  parseType = null;
  parseProtoType = null;
  versionName = null;
  errorHandler = null;
  curInputSource = null;
  updateStarted = false;
}

HRCParserImpl::~HRCParserImpl()
{
  int idx;
  for(FileTypeImpl *ft = fileTypeHash.enumerate(); ft != null; ft = fileTypeHash.next())
    delete ft;
  for(SchemeImpl *scheme = schemeHash.enumerate(); scheme != null; scheme = schemeHash.next()){
    delete scheme;
  };
  for(idx = 0; idx < regionNamesVector.size(); idx++)
    delete regionNamesVector.elementAt(idx);
  for(String *se = schemeEntitiesHash.enumerate(); se; se = schemeEntitiesHash.next()){
    delete se;
  };
  delete versionName;
}

void HRCParserImpl::setErrorHandler(colorer::ErrorHandler *eh){
  errorHandler = eh;
}

void HRCParserImpl::loadSource(XmlInputSource *is){
  XmlInputSource *istemp = curInputSource;
  curInputSource = is;
  if (is == null){
    if (errorHandler != null){
      errorHandler->error(StringBuffer("Can't open stream for type without location attribute"));
    }
    return;
  }
  try{
    parseHRC(is);
  }catch(Exception &){
    curInputSource = istemp;
    throw;
  }
  curInputSource = istemp;
}

void HRCParserImpl::unloadFileType(FileTypeImpl *filetype)
{
	bool loop=true;
	while (loop){
		loop=false;
		for(SchemeImpl *scheme = schemeHash.enumerate(); scheme != null; scheme = schemeHash.next()){
			if (scheme->fileType==filetype) {
				schemeHash.remove(scheme->getName());
				delete scheme;
				loop= true;
				break;
			}
		};
	}
	fileTypeVector.removeElement(filetype);
	fileTypeHash.remove(filetype->getName());
	delete filetype;
}

void HRCParserImpl::loadFileType(FileType *filetype)
{
  if (filetype == null) return;

  FileTypeImpl *thisType = (FileTypeImpl*)filetype;
  
  if (thisType->typeLoaded || thisType->inputSourceLoading || thisType->loadBroken){
    return;
  }
  thisType->inputSourceLoading = true;

  try{

    loadSource(thisType->inputSource);

  }catch(InputSourceException &e){
    if (errorHandler != null){
      errorHandler->fatalError(StringBuffer("Can't open source stream: ")+e.getMessage());
    }
    thisType->loadBroken = true;
  }catch(HRCParserException &e){
    if (errorHandler != null){
      errorHandler->fatalError(StringBuffer(e.getMessage())+" ["+DString(thisType->inputSource->getInputSource()->getSystemId())+"]");
    }
    thisType->loadBroken = true;
  }catch(Exception &e){
    if (errorHandler != null){
      errorHandler->fatalError(StringBuffer(e.getMessage())+" ["+DString(thisType->inputSource->getInputSource()->getSystemId())+"]");
    }
    thisType->loadBroken = true;
  }catch(...){
    if (errorHandler != null){
      errorHandler->fatalError(StringBuffer("Unknown exception while loading ")+DString(thisType->inputSource->getInputSource()->getSystemId()));
    }
    thisType->loadBroken = true;
  }
}

FileType *HRCParserImpl::chooseFileType(const String *fileName, const String *firstLine, int typeNo)
{
FileTypeImpl *best = null;
double max_prior = 0;
const double DELTA = 1e-6;
  for(int idx = 0; idx < fileTypeVector.size(); idx++){
    FileTypeImpl *ret = fileTypeVector.elementAt(idx);
    double prior = ret->getPriority(fileName, firstLine);

    if (typeNo > 0 && (prior-max_prior < DELTA)){
      best = ret;
      typeNo--;
    }
    if (prior-max_prior > DELTA || best == null){
      best = ret;
      max_prior = prior;
    }
  }
  if (typeNo > 0) return null;
  return best;
}

FileType *HRCParserImpl::getFileType(const String *name) {
  if (name == null) return null;
  return fileTypeHash.get(name);
}

FileType *HRCParserImpl::enumerateFileTypes(int index) {
  if (index < fileTypeVector.size()) return fileTypeVector.elementAt(index);
  return null;
}

int HRCParserImpl::getFileTypesCount() {
  return fileTypeVector.size();
}

int HRCParserImpl::getRegionCount() {
  return regionNamesVector.size();
}

const Region *HRCParserImpl::getRegion(int id) {
  if (id < 0 || id >= regionNamesVector.size()){
    return null;
  }
  return regionNamesVector.elementAt(id);
}

const Region* HRCParserImpl::getRegion(const String *name) {
  if (name == null) return null;
  return getNCRegion(name, false); // regionNamesHash.get(name);
}

const String *HRCParserImpl::getVersion() {
  return versionName;
}


// protected methods


void HRCParserImpl::parseHRC(XmlInputSource *is)
{
  xercesc::XercesDOMParser xml_parser;
  XmlParserErrorHandler error_handler(errorHandler);
  BaseEntityResolver resolver;
  xml_parser.setErrorHandler(&error_handler);
  xml_parser.setXMLEntityResolver(&resolver);
  xml_parser.setLoadExternalDTD(false);
  xml_parser.setSkipDTDValidation(true);
  xml_parser.parse(*is->getInputSource());
  if (error_handler.getSawErrors()) {
    throw HRCParserException(DString("Bad structure of hrc file."));
  }
  xercesc::DOMDocument *doc = xml_parser.getDocument();
  xercesc::DOMElement *root = doc->getDocumentElement();

  if (root && !xercesc::XMLString::equals(root->getNodeName(), hrcTagHrc)) {
    throw HRCParserException(StringBuffer("Bad hrc structure. Main '<hrc>' block not found.")+ CURRENT_FILE);
  }
  if (versionName == null)
    versionName = new DString(root->getAttribute(hrcHrcAttrVersion));

  bool globalUpdateStarted = false;
  if (!updateStarted){
    globalUpdateStarted = true;
    updateStarted = true;
  };

  parseHrcBlock(root);

  structureChanged = true;
  if (globalUpdateStarted){
    updateLinks();
    updateStarted = false;
  };
}

void HRCParserImpl::parseHrcBlock(const xercesc::DOMElement *elem)
{
  for (xercesc::DOMNode *node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement *elem = static_cast<xercesc::DOMElement*>(node);
      parseHrcBlockElements(elem);
      continue;
    }
    if (node->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      for (xercesc::DOMNode *sub_node = node->getFirstChild(); sub_node != nullptr; sub_node = sub_node->getNextSibling()) {
        xercesc::DOMElement *elem = static_cast<xercesc::DOMElement*>(sub_node);
        parseHrcBlockElements(elem);
      }
    }
  }
}

void HRCParserImpl::parseHrcBlockElements(const xercesc::DOMElement *elem)
{
  if (elem->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
    if (xercesc::XMLString::equals(elem->getNodeName(), hrcTagPrototype)) {
      addPrototype(elem);
      return;
    };
    if (xercesc::XMLString::equals(elem->getNodeName(), hrcTagPackage)) {
      addPrototype(elem);
      return;
    };
    if (xercesc::XMLString::equals(elem->getNodeName(), hrcTagType)) {
      addType(elem);
      return;
    };
    if (xercesc::XMLString::equals(elem->getNodeName(), hrcTagAnnotation)) {
      // not read anotation
      return;
    };
    errorHandler->warning(StringBuffer("Unused element '") + DString(elem->getNodeName()) + DString("'.") + CURRENT_FILE);
  }
}

void HRCParserImpl::addPrototype(const xercesc::DOMElement *elem)
{
  const XMLCh *typeName = elem->getAttribute(hrcPrototypeAttrName);
  const XMLCh *typeGroup = elem->getAttribute(hrcPrototypeAttrGroup);
  const XMLCh *typeDescription = elem->getAttribute(hrcPrototypeAttrDescription);
  if (*typeName == '\0'){
    if (errorHandler != null) errorHandler->error(DString("unnamed prototype "));
    return;
  }
  if (typeDescription == null){
    typeDescription = typeName;
  }
	FileTypeImpl* f=fileTypeHash.get(&DString(typeName));
  if (f != null){
		unloadFileType(f);
    if (errorHandler != null){
      errorHandler->warning(StringBuffer("Duplicate prototype '")+DString(typeName)+"'");
    }
  //  return;
  };
  FileTypeImpl *type = new FileTypeImpl(this);
  type->name = new SString(DString(typeName));
  type->description = new SString(DString(typeDescription));
  if (typeGroup != null){
    type->group = new SString(DString(typeGroup));
  }
  if (xercesc::XMLString::equals(elem->getNodeName(), hrcTagPackage)){
    type->isPackage = true;
  }
  parseProtoType = type;
  parsePrototypeBlock(elem);

  type->protoLoaded = true;
  fileTypeHash.put(type->name, type);
  if (!type->isPackage){
    fileTypeVector.addElement(type);
  };
}
void HRCParserImpl::parsePrototypeBlock(const xercesc::DOMElement *elem)
{
  for (xercesc::DOMNode *node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement *subelem = static_cast<xercesc::DOMElement*>(node);
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagLocation)){
        addPrototypeLocation(subelem);
        continue;
      };
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagFilename) ||
			        xercesc::XMLString::equals(subelem->getNodeName(), hrcTagFirstline)){
        addPrototypeDetectParam(subelem);
        continue;
      };
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagParametrs)){
        addPrototypeParameters(subelem);
        continue;
      };
    };
  };
}

void HRCParserImpl::addPrototypeLocation(const xercesc::DOMElement *elem)
{
  const XMLCh *locationLink = elem->getAttribute(hrcLocationAttrLink);
  if (*locationLink == '\0'){
    if (errorHandler != null){
      errorHandler->error(StringBuffer("Bad 'location' link attribute in prototype '")+parseProtoType->name+"'");
      return;
    }
  };
  parseProtoType->inputSource = XmlInputSource::newInstance(locationLink, curInputSource);
}

void HRCParserImpl::addPrototypeDetectParam(const xercesc::DOMElement *elem)
{
  if (elem->getFirstChild() == null || elem->getFirstChild()->getNodeType() != xercesc::DOMNode::TEXT_NODE){
    if (errorHandler != null) errorHandler->warning(StringBuffer("Bad '")+DString(elem->getNodeName())+"' element in prototype '"+parseProtoType->name+"'");
    return;
  };
  const XMLCh *match = ((xercesc::DOMText*)elem->getFirstChild())->getData();
  CRegExp *matchRE = new CRegExp(&DString(match));
  matchRE->setPositionMoves(true);
  if (!matchRE->isOk()){
    if (errorHandler != null){
      errorHandler->warning(StringBuffer("Fault compiling chooser RE '")+DString(match)+"' in prototype '"+parseProtoType->name+"'");
    }
    delete matchRE;
    return;
  };
  int ctype = xercesc::XMLString::equals(elem->getNodeName() , hrcTagFilename) ? 0 : 1;
  double prior = ctype ? 1 : 2;
  UnicodeTools::getNumber(&DString(elem->getAttribute(hrcFilenameAttrWeight)), &prior);
  FileTypeChooser *ftc = new FileTypeChooser(ctype, prior, matchRE);
  parseProtoType->chooserVector.addElement(ftc);
}

void HRCParserImpl::addPrototypeParameters(const xercesc::DOMElement *elem)
{
  for (xercesc::DOMNode *node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement *subelem = static_cast<xercesc::DOMElement*>(node);
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagParam)) {
        const XMLCh *name = subelem->getAttribute(hrcParamAttrName);
        const XMLCh *value = subelem->getAttribute(hrcParamAttrValue);
        const XMLCh *descr = subelem->getAttribute(hrcParamAttrDescription);
        if (*name == '\0' || *value == '\0'){
          if (errorHandler != null){
            errorHandler->warning(StringBuffer("Bad parameter in prototype '")+parseProtoType->name+"'");
          }
          continue;
        };
        TypeParameter* tp = parseProtoType->addParam(&DString(name));
        tp->default_value = new SString(DString(value));
        if (*descr != '\0'){
          tp->description = new SString(DString(descr));
        }
      };
      continue;
    };
    if (node->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      addPrototypeParameters(static_cast<xercesc::DOMElement*>(node));
    }
  };
}

void HRCParserImpl::addType(const xercesc::DOMElement *elem)
{
  const XMLCh *typeName = elem->getAttribute(hrcTypeAttrName);

  if (*typeName == '\0'){
    if (errorHandler != null){
      errorHandler->error(DString("Unnamed type found"));
    }
    return;
  };
  FileTypeImpl *type_ref = fileTypeHash.get(&DString(typeName));
  if (type_ref == null){
    if (errorHandler != null){
      errorHandler->error(StringBuffer("type '")+DString(typeName)+"' without prototype");
    }
    return;
  };
  FileTypeImpl *type = type_ref;
  if (type->typeLoaded){
    if (errorHandler != null) errorHandler->warning(StringBuffer("type '")+DString(typeName)+"' is already loaded");
    return;
  };
  type->typeLoaded = true;

  FileTypeImpl *o_parseType = parseType;
  parseType = type;

  parseTypeBlock(elem);

  String *baseSchemeName = qualifyOwnName(type->name);
  if (baseSchemeName != null){
    type->baseScheme = schemeHash.get(baseSchemeName);
  }
  delete baseSchemeName;
  if (type->baseScheme == null && !type->isPackage){
    if (errorHandler != null){
      errorHandler->warning(StringBuffer("type '")+DString(typeName)+"' has no default scheme");
    }
  }
  type->loadDone = true;
  parseType = o_parseType;
}

void HRCParserImpl::parseTypeBlock(const xercesc::DOMElement *elem)
{
  for (xercesc::DOMNode *node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement *subelem = static_cast<xercesc::DOMElement*>(node);
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagRegion)) {
        addTypeRegion(subelem);
        continue;
      };
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
      };
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagAnnotation)) {
        // not read anotation
        continue;
      };
    }
    // случай entity ссылки на другой файл.
    if (node->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      parseTypeBlock(static_cast<xercesc::DOMElement*>(node));
    }
  }
}

void HRCParserImpl::addTypeRegion(const xercesc::DOMElement *elem)
{
  const XMLCh *regionName = elem->getAttribute(hrcRegionAttrName);
  const XMLCh *regionParent = elem->getAttribute(hrcRegionAttrParent);
  const XMLCh *regionDescr = elem->getAttribute(hrcRegionAttrDescription);
  if (*regionName == '\0'){
    if (errorHandler != null) errorHandler->error(DString("No 'name' attribute in <region> element"));
    return;
  };
  String *qname1 = qualifyOwnName(&DString(regionName));
  if (qname1 == null) return;
  String *qname2 = qualifyForeignName(*regionParent!='\0'?&DString(regionParent):null, QNT_DEFINE, true);
  if (regionNamesHash.get(qname1) != null){
    if (errorHandler != null){
      errorHandler->warning(StringBuffer("Duplicate region '") + qname1 + "' definition in type '"+parseType->getName()+"'");
    }
    delete qname1;
    delete qname2;
    return;
  };

  const Region *region = new Region(qname1, &DString(regionDescr), getRegion(qname2), regionNamesVector.size());
  regionNamesVector.addElement(region);
  regionNamesHash.put(qname1, region);

  delete qname1;
  delete qname2;
}

void HRCParserImpl::addTypeEntity(const xercesc::DOMElement *elem)
{
  const XMLCh *entityName  = elem->getAttribute(hrcEntityAttrName);
  const XMLCh *entityValue = elem->getAttribute(hrcEntityAttrValue);
  if (*entityName == '\0' || elem->getAttributeNode(hrcEntityAttrValue) == null){
    if (errorHandler != null){
      errorHandler->error(DString("Bad entity attributes"));
    }
    return;
  };
  String *qname1 = qualifyOwnName(&DString(entityName));
  String *qname2 = useEntities(&DString(entityValue));
  if (qname1 != null && qname2 != null){
    schemeEntitiesHash.put(qname1, qname2);
    delete qname1;
  };
}

void HRCParserImpl::addTypeImport(const xercesc::DOMElement *elem)
{
  const XMLCh *typeParam = elem->getAttribute(hrcImportAttrType);
  if (*typeParam == '\0' || fileTypeHash.get(&DString(typeParam)) == null){
    if (errorHandler != null){
      errorHandler->error(StringBuffer("Import with bad '")+DString(typeParam)+"' attribute in type '"+parseType->name+"'");
    }
    return;
  };
  parseType->importVector.addElement(new SString(DString(typeParam)));
}

void HRCParserImpl::addScheme(const xercesc::DOMElement *elem)
{
  const XMLCh *schemeName = elem->getAttribute(hrcSchemeAttrName);
  String *qSchemeName = qualifyOwnName(*schemeName!='\0'?&DString(schemeName):null);
  if (qSchemeName == null){
    if (errorHandler != null) errorHandler->error(StringBuffer("bad scheme name in type '")+parseType->getName()+"'");
    return;
  }
  if (schemeHash.get(qSchemeName) != null ||
      disabledSchemes.get(qSchemeName) != 0){
    if (errorHandler != null) errorHandler->error(StringBuffer("duplicate scheme name '")+qSchemeName+"'");
    delete qSchemeName;
    return;
  }

  SchemeImpl *scheme = new SchemeImpl(qSchemeName);
  delete qSchemeName;
  scheme->fileType = parseType;

  schemeHash.put(scheme->schemeName, scheme);
  const XMLCh *condIf = elem->getAttribute(hrcSchemeAttrIf);
  const XMLCh *condUnless = elem->getAttribute(hrcSchemeAttrUnless);
  const String* p1 = parseType->getParamValue(DString(condIf));
  const String* p2 = parseType->getParamValue(DString(condUnless));
  if ((*condIf != '\0' && !DString("true").equals(p1)) ||
      (*condUnless != '\0' && DString("true").equals(p2))){
    //disabledSchemes.put(scheme->schemeName, 1);
    return;
  }
  parseSchemeBlock(scheme, elem);
}

void HRCParserImpl::parseSchemeBlock(SchemeImpl *scheme, const xercesc::DOMElement *elem) {
  for (xercesc::DOMNode *node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement *subelem = static_cast<xercesc::DOMElement*>(node);
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagInherit)) {
        addSchemeInherit(scheme, subelem);
        continue;
      };
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagRegexp)) {
        addSchemeRegexp(scheme, subelem);
        continue;
      };
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagBlock)) {
        addSchemeBlock(scheme, subelem);
        continue;
      };
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagKeywords)) {
        addSchemeKeywords(scheme, subelem);
        continue;
      };
      if (xercesc::XMLString::equals(subelem->getNodeName(), hrcTagAnnotation)) {
        // not read anotation
        continue;
      };
    }
    if (node->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      parseSchemeBlock(scheme, static_cast<xercesc::DOMElement*>(node));
    }
  }
}

void HRCParserImpl::addSchemeInherit(SchemeImpl *scheme, const xercesc::DOMElement *elem)
{
  const XMLCh *nqSchemeName = elem->getAttribute(hrcInheritAttrScheme);
  if (*nqSchemeName == '\0'){
    if (errorHandler != null){
      errorHandler->error(StringBuffer("empty scheme name in inheritance operator in scheme '")+scheme->schemeName+"'");
    }
    return;
  };
  SchemeNode *scheme_node = new SchemeNode();
  scheme_node->type = SNT_INHERIT;
  scheme_node->schemeName = new SString(DString(nqSchemeName));
  String *schemeName = qualifyForeignName(&DString(nqSchemeName), QNT_SCHEME, false);
  if (schemeName == null){
    //        if (errorHandler != null) errorHandler->warning(StringBuffer("forward inheritance of '")+nqSchemeName+"'. possible inherit loop with '"+scheme->schemeName+"'");
    //        delete next;
    //        continue;
  }else
    scheme_node->scheme = schemeHash.get(schemeName);
  if (schemeName != null){
    delete scheme_node->schemeName;
    scheme_node->schemeName = schemeName;
  };

  for (xercesc::DOMNode *node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement *subelem = static_cast<xercesc::DOMElement*>(node);
      if (xercesc::XMLString::equals(subelem->getNodeName() , hrcTagVirtual)) {
        const XMLCh *schemeName = subelem->getAttribute(hrcVirtualAttrScheme);
        const XMLCh *substName = subelem->getAttribute(hrcVirtualAttrSubstScheme);
        if (*schemeName == '\0' || *substName == '\0'){
          if (errorHandler != null){
            errorHandler->error(StringBuffer("bad virtualize attributes in scheme '")+scheme->schemeName+"'");
          }
          continue;
        };
        scheme_node->virtualEntryVector.addElement(new VirtualEntry(&DString(schemeName), &DString(substName)));
      };
    };
  };
  scheme->nodes.addElement(scheme_node);
}

void HRCParserImpl::addSchemeRegexp(SchemeImpl *scheme, const xercesc::DOMElement *elem)
{
  const XMLCh *matchParam = elem->getAttribute(hrcRegexpAttrMatch);
  if (*matchParam == '\0'){
    for (xercesc::DOMNode *child = elem->getFirstChild(); child!=nullptr; child = child->getNextSibling()) {
      if(child->getNodeType() == xercesc::DOMNode::CDATA_SECTION_NODE){
        matchParam = ((xercesc::DOMCDATASection*)child)->getData();
        break;
      }
      if(child->getNodeType() == xercesc::DOMNode::TEXT_NODE){
        const XMLCh *matchParam1;
        matchParam1 = ((xercesc::DOMText*)child)->getData();
        xercesc::XMLString::trim((XMLCh*)matchParam1);
        if (*matchParam1!='\0') {
          matchParam = matchParam1;
          break;
        }
      }
    }
  }
  if (matchParam == null){
    if (errorHandler != null){
      errorHandler->error(StringBuffer("no 'match' in regexp in scheme ")+scheme->schemeName);
    }
    return;
  };
  String *entMatchParam = useEntities(&DString(matchParam));
  SchemeNode *scheme_node = new SchemeNode();
  scheme_node->lowPriority = DString("low").equals(&DString(elem->getAttribute(hrcRegexpAttrPriority)));
  scheme_node->type = SNT_RE;
  scheme_node->start = new CRegExp(entMatchParam);
  if (!scheme_node->start || !scheme_node->start->isOk())
    if (errorHandler != null) errorHandler->error(StringBuffer("fault compiling regexp '")+entMatchParam+"' in scheme '"+scheme->schemeName+"'");
  delete entMatchParam;
  scheme_node->start->setPositionMoves(false);
  scheme_node->end = 0;

  loadRegions(scheme_node, elem, true);
  if(scheme_node->region){
    scheme_node->regions[0] = scheme_node->region;
  }

  scheme->nodes.addElement(scheme_node);
}

void HRCParserImpl::addSchemeBlock(SchemeImpl *scheme, const xercesc::DOMElement *elem)
{
  const XMLCh *sParam = elem->getAttribute(hrcBlockAttrStart);
  const XMLCh *eParam = elem->getAttribute(hrcBlockAttrEnd);

  xercesc::DOMElement *eStart = NULL, *eEnd = NULL;

  for(xercesc::DOMNode *blkn = elem->getFirstChild(); blkn && !(*eParam!='\0' && *sParam!='\0'); blkn = blkn->getNextSibling())
  {
    xercesc::DOMElement *blkel;
    if(blkn->getNodeType() == xercesc::DOMNode::ELEMENT_NODE){
      blkel = static_cast<xercesc::DOMElement*>(blkn);
    }
    else continue;

    const XMLCh *p='\0';
    if (blkel->getAttributeNode(hrcBlockAttrMatch)){
      p = blkel->getAttribute(hrcBlockAttrMatch);
    }else{
      for (xercesc::DOMNode *child = blkel->getFirstChild(); child!=nullptr; child = child->getNextSibling()) {
        if(child->getNodeType() == xercesc::DOMNode::CDATA_SECTION_NODE){
          p = ((xercesc::DOMCDATASection*)child)->getData();
          break;
        }
        if(child->getNodeType() == xercesc::DOMNode::TEXT_NODE){
          const XMLCh *p1;
          p1 = ((xercesc::DOMText*)child)->getData();
          xercesc::XMLString::trim((XMLCh*)p1);
          if (*p1!='\0') {
            p = p1;
            break;
          }
        }
      }
    }

    if(xercesc::XMLString::equals(blkel->getNodeName() , hrcBlockAttrStart))
    {
      sParam = p;
      eStart = blkel;
    }
    if(xercesc::XMLString::equals(blkel->getNodeName() , hrcBlockAttrEnd))
    {
      eParam = p;
      eEnd = blkel;
    }
  }

  String *startParam;
  String *endParam;
  if (!(startParam = useEntities(&DString(sParam)))){
    if (errorHandler != null){
      errorHandler->error(StringBuffer("'start' block attribute not found in scheme '")+scheme->schemeName+"'");
    }
    delete startParam;
    return;
  }
  if (!(endParam = useEntities(&DString(eParam)))){
    if (errorHandler != null){
      errorHandler->error(StringBuffer("'end' block attribute not found in scheme '")+scheme->schemeName+"'");
    }
    delete startParam;
    delete endParam;
    return;
  }
  const XMLCh *schemeName = elem->getAttribute(hrcBlockAttrScheme);
  if (*schemeName == '\0'){
    if (errorHandler != null){
      errorHandler->error(StringBuffer("block with bad scheme attribute in scheme '")+scheme->getName()+"'");
    }
    delete startParam;
    delete endParam;
    return;
  }
  SchemeNode *scheme_node = new SchemeNode();
  scheme_node->schemeName = new SString(DString(schemeName));
  scheme_node->lowPriority = DString("low").equals(&DString(elem->getAttribute(hrcBlockAttrPriority)));
  scheme_node->lowContentPriority = DString("low").equals(&DString(elem->getAttribute(hrcBlockAttrContentPriority)));
  scheme_node->innerRegion = DString("yes").equals(&DString(elem->getAttribute(hrcBlockAttrInnerRegion)));
  scheme_node->type = SNT_SCHEME;
  scheme_node->start = new CRegExp(startParam);
  scheme_node->start->setPositionMoves(false);
  if (!scheme_node->start->isOk()){
    if (errorHandler != null){
      errorHandler->error(StringBuffer("fault compiling regexp '")+startParam+"' in scheme '"+scheme->schemeName+"'");
    }
  }
  scheme_node->end = new CRegExp();
  scheme_node->end->setPositionMoves(true);
  scheme_node->end->setBackRE(scheme_node->start);
  scheme_node->end->setRE(endParam);
  if (!scheme_node->end->isOk()){
    if (errorHandler != null){
      errorHandler->error(StringBuffer("fault compiling regexp '")+endParam+"' in scheme '"+scheme->schemeName+"'");
    }
  }
  delete startParam;
  delete endParam;

  // !! EE
  loadBlockRegions(scheme_node, elem);
  loadRegions(scheme_node, eStart, true);
  loadRegions(scheme_node, eEnd, false);
  scheme->nodes.addElement(scheme_node);
}

void HRCParserImpl::addSchemeKeywords(SchemeImpl *scheme, const xercesc::DOMElement *elem)
{
  SchemeNode *scheme_node = new SchemeNode();
  bool isCase = !DString("yes").equals(&DString(elem->getAttribute(hrcKeywordsAttrIgnorecase)));
  scheme_node->lowPriority = !DString("normal").equals(&DString(elem->getAttribute(hrcKeywordsAttrPriority)));
  const Region *brgn = getNCRegion(elem, DString("region"));
  if (brgn == null){
    return;
  }
  const XMLCh *worddiv = elem->getAttribute(hrcKeywordsAttrWorddiv);

  scheme_node->worddiv = null;
  if (*worddiv!='\0'){
    String *entWordDiv = useEntities(&DString(worddiv));
    scheme_node->worddiv = CharacterClass::createCharClass(*entWordDiv, 0, null);
    if(scheme_node->worddiv == null){
      if (errorHandler != null) errorHandler->warning(StringBuffer("fault compiling worddiv regexp '")+entWordDiv+"' in scheme '"+scheme->schemeName+"'");
    }
    delete entWordDiv;
  };

  scheme_node->kwList = new KeywordList;
  scheme_node->kwList->num = getSchemeKeywordsCount(elem);

  scheme_node->kwList->kwList = new KeywordInfo[scheme_node->kwList->num];
  memset(scheme_node->kwList->kwList ,0,sizeof(KeywordInfo)*scheme_node->kwList->num);
  scheme_node->kwList->num = 0;
  scheme_node->kwList->matchCase = isCase;
  scheme_node->type = SNT_KEYWORDS;

  for(xercesc::DOMNode *keywrd = elem->getFirstChild(); keywrd; keywrd = keywrd->getNextSibling()){
    if (keywrd->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      addKeyword(scheme_node, brgn, static_cast<xercesc::DOMElement*>(keywrd));
      continue;
    }
    if (keywrd->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      for(xercesc::DOMNode *keywrd2 = keywrd->getFirstChild(); keywrd2; keywrd2 = keywrd2->getNextSibling()){
        if (keywrd2->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
          addKeyword(scheme_node, brgn, static_cast<xercesc::DOMElement*>(keywrd2));
        }
      }
    }
  };
  scheme_node->kwList->sortList();
  scheme_node->kwList->substrIndex();
  scheme->nodes.addElement(scheme_node);
}

void HRCParserImpl::addKeyword(SchemeNode* scheme_node, const Region *brgn, const xercesc::DOMElement *elem)
{
  int type = 0;
  if (xercesc::XMLString::equals(elem->getNodeName() , hrcTagWord)) type = 1;
  if (xercesc::XMLString::equals(elem->getNodeName() , hrcTagSymb)) type = 2;
  if (!type){
    return;
  }
  const XMLCh *param = elem->getAttribute(hrcWordAttrName);
  if (*param =='\0'){
    return;
  }

  const Region *rgn = brgn;
  const XMLCh *reg=elem->getAttribute(hrcWordAttrRegion);
  if (*reg!='\0')
    rgn = getNCRegion(elem, DString(hrcWordAttrRegion));

  int pos = scheme_node->kwList->num;
  scheme_node->kwList->kwList[pos].keyword = new SString(DString(param));
  scheme_node->kwList->kwList[pos].region = rgn;
  scheme_node->kwList->kwList[pos].isSymbol = (type == 2);
  scheme_node->kwList->kwList[pos].ssShorter = -1;
  scheme_node->kwList->firstChar->addChar(param[0]);
  if (!scheme_node->kwList->matchCase){
    scheme_node->kwList->firstChar->addChar(Character::toLowerCase(param[0]));
    scheme_node->kwList->firstChar->addChar(Character::toUpperCase(param[0]));
    scheme_node->kwList->firstChar->addChar(Character::toTitleCase(param[0]));
  };
  scheme_node->kwList->num++;
  if (scheme_node->kwList->minKeywordLength > scheme_node->kwList->kwList[pos].keyword->length())
    scheme_node->kwList->minKeywordLength = scheme_node->kwList->kwList[pos].keyword->length();
}

int HRCParserImpl::getSchemeKeywordsCount(const xercesc::DOMElement *elem)
{
  int result = 0;
  for(xercesc::DOMNode *keywrd_count = elem->getFirstChild(); keywrd_count; keywrd_count = keywrd_count->getNextSibling()){
    if (keywrd_count->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      if (xercesc::XMLString::equals(keywrd_count->getNodeName() , hrcTagWord) ||
        xercesc::XMLString::equals(keywrd_count->getNodeName() , hrcTagSymb))
      {
        result++;
      }
      continue;
    }
    if (keywrd_count->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      result +=getSchemeKeywordsCount(static_cast<xercesc::DOMElement*>(keywrd_count));
    }
  }
  return result;
}

void HRCParserImpl::loadRegions(SchemeNode *node, const xercesc::DOMElement *el, bool st)
{
  static char rg_tmpl[8] = "region\0";

  if(el)
  {
    if (node->region == null){
      node->region = getNCRegion(el, DString("region"));
    }

    for(int i = 0; i < REGIONS_NUM; i++)
    {
      rg_tmpl[6] = (i < 0xA ? i : i+7+32) + '0';

      if(st){
        node->regions[i] = getNCRegion(el, DString(rg_tmpl));
      }
      else{
        node->regione[i] = getNCRegion(el, DString(rg_tmpl));
      }
    }
  }

  for (int i = 0; i < NAMED_REGIONS_NUM; i++)
  {
    if(st) node->regionsn[i] = getNCRegion(node->start->getBracketName(i), false);
    else   node->regionen[i] = getNCRegion(node->end->getBracketName(i), false);
  }
}

void HRCParserImpl::loadBlockRegions(SchemeNode *node, const xercesc::DOMElement *el)
{
  int i;
  static char rg_tmpl[9] = "region\0\0";

  node->region = getNCRegion(el, DString("region"));
  for (i = 0; i < REGIONS_NUM; i++){
    rg_tmpl[6] = '0'; rg_tmpl[7] = (i<0xA?i:i+7+32)+'0';
    node->regions[i] = getNCRegion(el, DString(rg_tmpl));
    rg_tmpl[6] = '1';
    node->regione[i] = getNCRegion(el, DString(rg_tmpl));
  }
}

void HRCParserImpl::updateLinks()
{
  while(structureChanged){
    structureChanged = false;
    for(SchemeImpl *scheme = schemeHash.enumerate(); scheme != null; scheme = schemeHash.next()){
      if (!scheme->fileType->loadDone) continue;
      FileTypeImpl *old_parseType = parseType;
      parseType = scheme->fileType;
      for (int sni = 0; sni < scheme->nodes.size(); sni++){
        SchemeNode *snode = scheme->nodes.elementAt(sni);
        if (snode->schemeName != null && (snode->type == SNT_SCHEME || snode->type == SNT_INHERIT) && snode->scheme == null){
          String *schemeName = qualifyForeignName(snode->schemeName, QNT_SCHEME, true);
          if (schemeName != null){
            snode->scheme = schemeHash.get(schemeName);
          }else{
            if (errorHandler != null) errorHandler->error(StringBuffer("cannot resolve scheme name '")+snode->schemeName+"' in scheme '"+scheme->schemeName+"'");
          };
          delete schemeName;
          delete snode->schemeName;
          snode->schemeName = null;
        };
        if (snode->type == SNT_INHERIT){
          for(int vti = 0; vti < snode->virtualEntryVector.size(); vti++){
            VirtualEntry *vt = snode->virtualEntryVector.elementAt(vti);
            if (vt->virtScheme == null && vt->virtSchemeName != null){
              String *vsn = qualifyForeignName(vt->virtSchemeName, QNT_SCHEME, true);
              if (vsn) vt->virtScheme = schemeHash.get(vsn);
              if (!vsn) if (errorHandler != null) errorHandler->error(StringBuffer("cannot virtualize scheme '")+vt->virtSchemeName+"' in scheme '"+scheme->schemeName+"'");
              delete vsn;
              delete vt->virtSchemeName;
              vt->virtSchemeName = null;
            };
            if (vt->substScheme == null && vt->substSchemeName != null){
              String *vsn = qualifyForeignName(vt->substSchemeName, QNT_SCHEME, true);
              if (vsn) vt->substScheme = schemeHash.get(vsn);
              else if (errorHandler != null) errorHandler->error(StringBuffer("cannot virtualize using subst-scheme scheme '")+vt->substSchemeName+"' in scheme '"+scheme->schemeName+"'");
              delete vsn;
              delete vt->substSchemeName;
              vt->substSchemeName = null;
            };
          };
        };
      };
      parseType = old_parseType;
      if (structureChanged) break;
    };
  };
};

String *HRCParserImpl::qualifyOwnName(const String *name) {
  if (name == null) return null;
  int colon = name->indexOf(':');
  if (colon != -1){
    if (parseType && DString(name, 0, colon) != *parseType->name){
      if (errorHandler != null) errorHandler->error(StringBuffer("type name qualifer in '")+name+"' doesn't match type '"+parseType->name+"'");
      return null;
    }else return new SString(name);
  }else{
    if (parseType == null) return null;
    StringBuffer *sbuf = new StringBuffer(parseType->name);
    sbuf->append(DString(":")).append(name);
    return sbuf;
  }
}

bool HRCParserImpl::checkNameExist(const String *name, FileTypeImpl *parseType, QualifyNameType qntype, bool logErrors) {
  if (qntype == QNT_DEFINE && regionNamesHash.get(name) == null){
    if (logErrors)
      if (errorHandler != null) errorHandler->error(StringBuffer("region '")+name+"', referenced in type '"+parseType->name+"', is not defined");
    return false;
  }else if (qntype == QNT_ENTITY && schemeEntitiesHash.get(name) == null){
    if (logErrors)
      if (errorHandler != null) errorHandler->error(StringBuffer("entity '")+name+"', referenced in type '"+parseType->name+"', is not defined");
    return false;
  }else if (qntype == QNT_SCHEME && schemeHash.get(name) == null){
    if (logErrors)
      if (errorHandler != null) errorHandler->error(StringBuffer("scheme '")+name+"', referenced in type '"+parseType->name+"', is not defined");
    return false;
  };
  return true;
}

String *HRCParserImpl::qualifyForeignName(const String *name, QualifyNameType qntype, bool logErrors){
  if (name == null) return null;
  int colon = name->indexOf(':');
  if (colon != -1){ // qualified name
    DString prefix(name, 0, colon);
    FileTypeImpl *prefType = fileTypeHash.get(&prefix);

    if (prefType == null){
      if (logErrors && errorHandler != null) errorHandler->error(StringBuffer("type name qualifer in '")+name+"' doesn't match any type");
      return null;
    }else
      if (!prefType->typeLoaded) loadFileType(prefType);
    if (prefType == parseType || prefType->typeLoaded)
      return checkNameExist(name, prefType, qntype, logErrors)?(new SString(name)):null;
  }else{ // unqualified name
    for(int idx = -1; parseType != null && idx < parseType->importVector.size(); idx++){
      const String *tname = parseType->name;
      if (idx > -1) tname = parseType->importVector.elementAt(idx);
      FileTypeImpl *importer = fileTypeHash.get(tname);
      if (!importer->typeLoaded) loadFileType(importer);

      StringBuffer *qname = new StringBuffer(tname);
      qname->append(DString(":")).append(name);
      if (checkNameExist(qname, importer, qntype, false)) return qname;
      delete qname;
    };
    if (logErrors && errorHandler != null){
      errorHandler->error(StringBuffer("unqualified name '")+name+"' doesn't belong to any imported type ["+DString(curInputSource->getInputSource()->getSystemId())+"]");
    }
  };
  return null;
}

String *HRCParserImpl::useEntities(const String *name)
{
  int copypos = 0;
  int epos = 0;

  if (!name) return null;
  StringBuffer *newname = new StringBuffer();

  while(true){
    epos = name->indexOf('%', epos);
    if (epos == -1){
      epos = name->length();
      break;
    };
    if (epos && (*name)[epos-1] == '\\'){
      epos++;
      continue;
    };
    int elpos = name->indexOf(';', epos);
    if (elpos == -1){
      epos = name->length();
      break;
    };
    DString enname(name, epos+1, elpos-epos-1);

    String *qEnName = qualifyForeignName(&enname, QNT_ENTITY, true);
    const String *enval = null;
    if (qEnName != null){
      enval = schemeEntitiesHash.get(qEnName);
      delete qEnName;
    };
    if (enval == null){
      epos++;
      continue;
    };
    newname->append(DString(name, copypos, epos-copypos));
    newname->append(enval);
    epos = elpos+1;
    copypos = epos;
  };
  if (epos > copypos) newname->append(DString(name, copypos, epos-copypos));
  return newname;
};

const Region* HRCParserImpl::getNCRegion(const String *name, bool logErrors){
  if (name == null) return null;
  const Region *reg = null;
  String *qname = qualifyForeignName(name, QNT_DEFINE, logErrors);
  if (qname == null) return null;
  reg = regionNamesHash.get(qname);
  delete qname;
  /** Check for 'default' region request.
      Regions with this name are always transparent
  */
  if (reg != null){
    const String *name = reg->getName();
    int idx = name->indexOf(DString(":default"));
    if (idx != -1  && idx+8 == name->length()) return null;
  };
  return reg;
};

const Region* HRCParserImpl::getNCRegion(const xercesc::DOMElement *el, const String &tag){
  const XMLCh *par = el->getAttribute(tag.getWChars());
  if (*par == '\0') return null;
  return getNCRegion(&DString(par), true);
};

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
