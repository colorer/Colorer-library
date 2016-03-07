#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <colorer/parsers/CatalogParser.h>
#include <colorer/xml/XmlInputSource.h>
#include <colorer/xml/XmlParserErrorHandler.h>
#include <colorer/xml/BaseEntityResolver.h>
#include <colorer/xml/XmlTagDefs.h>

void CatalogParser::parse(const String* path)
{
  LOG(DEBUG) << "begin parse catalog.xml";
  hrc_locations.clear();
  hrd_nodes.clear();

  xercesc::XercesDOMParser xml_parser;
  XmlParserErrorHandler error_handler;
  BaseEntityResolver resolver;
  xml_parser.setErrorHandler(&error_handler);
  xml_parser.setXMLEntityResolver(&resolver);
  xml_parser.setLoadExternalDTD(false);
  xml_parser.setSkipDTDValidation(true);
  uXmlInputSource catalogXIS = XmlInputSource::newInstance(path->getWChars(), static_cast<XMLCh*>(nullptr));
  xml_parser.parse(*catalogXIS->getInputSource());
  if (error_handler.getSawErrors()) {
    throw CatalogParserException("Error reading catalog.xml.");
  }
  xercesc::DOMDocument* catalog = xml_parser.getDocument();
  xercesc::DOMElement* elem = catalog->getDocumentElement();

  if (!elem || !xercesc::XMLString::equals(elem->getNodeName(), catTagCatalog)) {
    throw CatalogParserException(SString("Incorrect file structure catalog.xml. Main '<catalog>' block not found at file ") + path);
  }

  parseCatalogBlock(elem);

  LOG(DEBUG) << "end parse catalog.xml";
}

void CatalogParser::parseCatalogBlock(const xercesc::DOMElement* elem)
{
  for (xercesc::DOMNode* node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement* subelem = static_cast<xercesc::DOMElement*>(node);
      if (xercesc::XMLString::equals(subelem->getNodeName(), catTagHrcSets)) {
        parseHrcSetsBlock(subelem);
        continue;
      }
      if (xercesc::XMLString::equals(subelem->getNodeName(), catTagHrdSets)) {
        parseHrdSetsBlock(subelem);
      }
      continue;
    }
    if (node->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      parseCatalogBlock(static_cast<xercesc::DOMElement*>(node));
    }
  }
}

void CatalogParser::parseHrcSetsBlock(const xercesc::DOMElement* elem)
{
  addHrcSetsLocation(elem);
}

void CatalogParser::addHrcSetsLocation(const xercesc::DOMElement* elem)
{
  for (xercesc::DOMNode* node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement* subelem = static_cast<xercesc::DOMElement*>(node);
      if (xercesc::XMLString::equals(subelem->getNodeName(), catTagLocation)) {
        auto attr_value = subelem->getAttribute(catLocationAttrLink);
        if (*attr_value != xercesc::chNull) {
          hrc_locations.emplace_back(SString(attr_value));
          LOG(DEBUG) << "add hrc location: '" << hrc_locations.back().getChars() << "'";
        } else {
          LOG(WARNING) << "found hrc with empty location. skip it location.";
        }
      }
      continue;
    }
    if (node->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      addHrcSetsLocation(static_cast<xercesc::DOMElement*>(node));
    }
  }
}

void CatalogParser::parseHrdSetsBlock(const xercesc::DOMElement* elem)
{
  for (xercesc::DOMNode* node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      xercesc::DOMElement* subelem = static_cast<xercesc::DOMElement*>(node);
      if (xercesc::XMLString::equals(subelem->getNodeName(), catTagHrd)) {
        parseHRDSetsChild(subelem);
      }
      continue;
    }
    if (node->getNodeType() == xercesc::DOMNode::ENTITY_REFERENCE_NODE) {
      parseHrdSetsBlock(static_cast<xercesc::DOMElement*>(node));
    }
  }
}

void CatalogParser::parseHRDSetsChild(const xercesc::DOMElement* elem)
{
  const XMLCh* xhrd_class = elem->getAttribute(catHrdAttrClass);
  const XMLCh* xhrd_name = elem->getAttribute(catHrdAttrName);
  const XMLCh* xhrd_desc = elem->getAttribute(catHrdAttrDescription);

  if (*xhrd_class == xercesc::chNull || *xhrd_name == xercesc::chNull) {
    LOG(WARNING) << "found HRD with empty class/name. skip this record.";
    return;
  }

  auto hrd_node = std::make_unique<HRDNode>();
  hrd_node->hrd_class = SString(xhrd_class);
  hrd_node->hrd_name = SString(xhrd_name);
  hrd_node->hrd_description = SString(xhrd_desc);

  for (xercesc::DOMNode* node = elem->getFirstChild(); node != nullptr; node = node->getNextSibling()) {
    if (node->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      if (xercesc::XMLString::equals(node->getNodeName(), catTagLocation)) {
        xercesc::DOMElement* subelem = static_cast<xercesc::DOMElement*>(node);
        auto attr_value = subelem->getAttribute(catLocationAttrLink);
        if (*attr_value != xercesc::chNull) {
          hrd_node->hrd_location.emplace_back(SString(DString(attr_value)));
          LOGF(DEBUG, "add hrd location '%s' for %s:%s", hrd_node->hrd_location.back().getChars(), hrd_node->hrd_class.getChars(), hrd_node->hrd_name.getChars());
        } else {
          LOG(WARNING) << "found hrd with empty location. skip it location.";
        }
      }
    }
  }

  if (hrd_node->hrd_location.size() > 0) {
    hrd_nodes.push_back(std::move(hrd_node));
  } else {
    LOGF(WARNING, "skip HRD %s:%s - not found valid location", hrd_node->hrd_class.getChars(), hrd_node->hrd_name.getChars());
  }
}
