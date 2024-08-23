#include "LibXmlReader.h"

xmlExternalEntityLoader defaultLoader = NULL;

LibXmlReader::LibXmlReader(const UnicodeString& o) : doc_(nullptr)
{
  defaultLoader = xmlGetExternalEntityLoader();
 // xmlSetExternalEntityLoader(xmlMyExternalEntityLoader);
  doc_ = parse(o);
}

LibXmlReader::~LibXmlReader()
{
  if (doc_ != nullptr) {
    xmlFreeDoc(doc_);
  }
}

xmlDocPtr LibXmlReader::parse(const UnicodeString& file)
{
  return xmlReadFile(UStr::to_stdstr(&file).c_str(), nullptr, XML_PARSE_NOENT | XML_PARSE_NONET);
}

void LibXmlReader::parse(std::list<XMLNode>& nodes)
{
  xmlNode* current = xmlDocGetRootElement(doc_);
  while (current != nullptr) {
    XMLNode result;
    populateNode(current, result);
    nodes.push_back(result);
    current = current->next;
  }
}

/**
 * Utility methods
 * */
bool LibXmlReader::populateNode(xmlNode* node, XMLNode& result)
{
  if (node->type == XML_ELEMENT_NODE) {
    result.name = UnicodeString((const char*) node->name);

    // if (node->children != nullptr && node->children->next == nullptr &&
    //     (node->children->type == XML_CDATA_SECTION_NODE || node->children->type == XML_TEXT_NODE))
    // {
    //   result.text = UnicodeString((char*) xmlNodeGetContent(node->children));
    // }
    // else {
    //   getChildren(node, result);
    // }
    const auto t = getElementText(node);
    if (!t.isEmpty()) {
      result.text = t;
    }
    getChildren(node, result);
    getAttributes(node, result.attributes);

    return true;
  }
  if (node->type == XML_ENTITY_REF_NODE) {
    auto* p = xmlGetDocEntity(doc_, node->name);
    // xmlCtxtSetResourceLoader
    // xmlNewInputFromUrl
    result.name = UnicodeString((const char*) p->URI);
    return true;
  }
  return false;
}

UnicodeString LibXmlReader::getElementText(xmlNode* node)
{
  for (xmlNode* child = node->children; child != nullptr; child = child->next) {
    if (child->type == XML_CDATA_SECTION_NODE) {
      return UnicodeString((const char*) child->content);
    }
    if (child->type == XML_TEXT_NODE) {
      auto t = UnicodeString((const char*) child->content);
      t=t.trim();
      if (t.isEmpty()) continue;
      return UnicodeString((const char*) child->content);
    }
  }
  return UnicodeString("");
}

void LibXmlReader::getChildren(xmlNode* node, XMLNode& result)
{
  if (node->children == nullptr)
    return;
  node = node->children;

  while (node != nullptr) {
    if (!xmlIsBlankNode(node)) {
      XMLNode child;
      if (populateNode(node, child))
        result.children.push_back(child);
    }
    node = node->next;
  }
}

void LibXmlReader::getAttributes(const xmlNode* node, std::unordered_map<UnicodeString, UnicodeString>& data)
{
  for (xmlAttrPtr attr = node->properties; attr != nullptr; attr = attr->next) {
    data.insert(std::pair((const char*) attr->name, (const char*) xmlNodeGetContent(attr->children)));
  }
}

xmlParserInputPtr LibXmlReader::xmlMyExternalEntityLoader(const char* URL, const char* ID, xmlParserCtxtPtr ctxt)
{
  xmlParserInputPtr ret = nullptr;
  //const char* fileID = NULL;
  /* lookup for the fileID
   * The documentation suggests using the ID, but for me this was
   * always NULL so I had to lookup by URL instead.
   */

  // ret = xmlNewInputFromFile(ctxt, fileID);
  // if (ret != NULL)
  //     return (ret);
  if (defaultLoader != NULL)
    ret = defaultLoader(URL, ID, ctxt);
  return (ret);
}