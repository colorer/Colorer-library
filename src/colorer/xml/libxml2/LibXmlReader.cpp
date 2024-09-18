#include "colorer/xml/libxml2/LibXmlReader.h"
#include <libxml/parserInternals.h>
#include <cstring>
#include <fstream>
#include "colorer/Exception.h"

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
#include "colorer/zip/MemoryFile.h"

uUnicodeString LibXmlReader::current_jar = nullptr;
#endif

#ifdef _MSC_VER
#define strdup(p) _strdup(p)
#endif

LibXmlReader::LibXmlReader(const UnicodeString& source_file) : xmldoc(nullptr)
{
  xmlSetExternalEntityLoader(xmlMyExternalEntityLoader);
  xmlSetGenericErrorFunc(nullptr, xml_error_func);

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
  current_jar.reset();
  if (source_file.startsWith(jar)) {
    const auto path_idx = source_file.lastIndexOf('!');
    const UnicodeString path_in_jar(source_file, path_idx + 1);

    current_jar = std::make_unique<UnicodeString>(source_file, jar.length(), path_idx - jar.length());
    xmldoc = xmlReadFile(UStr::to_stdstr(&path_in_jar).c_str(), nullptr, XML_PARSE_NOENT | XML_PARSE_NONET);
    return;
  }
#endif
  xmldoc = xmlReadFile(UStr::to_stdstr(&source_file).c_str(), nullptr, XML_PARSE_NOENT | XML_PARSE_NONET);
}

LibXmlReader::LibXmlReader(const XmlInputSource& source) : LibXmlReader(source.getPath()) {}

LibXmlReader::~LibXmlReader()
{
  if (xmldoc != nullptr) {
    xmlFreeDoc(xmldoc);
  }
}

void LibXmlReader::parse(std::list<XMLNode>& nodes)
{
  xmlNode* current = xmlDocGetRootElement(xmldoc);
  while (current != nullptr) {
    XMLNode result;
    populateNode(current, result);
    nodes.push_back(result);
    current = current->next;
  }
}

bool LibXmlReader::populateNode(xmlNode* node, XMLNode& result)
{
  if (node->type == XML_ELEMENT_NODE) {
    result.name = UnicodeString((const char*) node->name);

    const auto text_string = getElementText(node);
    if (!text_string.isEmpty()) {
      result.text = text_string;
    }
    getChildren(node, result);
    getAttributes(node, result.attributes);

    return true;
  }
  return false;
}

UnicodeString LibXmlReader::getElementText(const xmlNode* node)
{
  for (const xmlNode* child = node->children; child != nullptr; child = child->next) {
    if (child->type == XML_CDATA_SECTION_NODE) {
      return UnicodeString((const char*) child->content);
    }
    if (child->type == XML_TEXT_NODE) {
      auto temp_string = UnicodeString((const char*) child->content);
      temp_string.trim();
      if (temp_string.isEmpty()) {
        continue;
      }
      return temp_string;
    }
  }
  return UnicodeString("");
}

void LibXmlReader::getChildren(xmlNode* node, XMLNode& result)
{
  if (node->children == nullptr) {
    return;
  }
  node = node->children;

  while (node != nullptr) {
    if (!xmlIsBlankNode(node)) {
      XMLNode child;
      if (populateNode(node, child)) {
        result.children.push_back(child);
      }
    }
    node = node->next;
  }
}

void LibXmlReader::getAttributes(const xmlNode* node, std::unordered_map<UnicodeString, UnicodeString>& data)
{
  for (xmlAttrPtr attr = node->properties; attr != nullptr; attr = attr->next) {
    auto content = xmlNodeGetContent(attr->children);
    data.emplace(std::pair((const char*) attr->name, (const char*) content));
    xmlFree(content);
  }
}

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
xmlParserInputPtr LibXmlReader::xmlZipEntityLoader(const char* URL, const xmlParserCtxtPtr ctxt)
{
  const UnicodeString path_in_jar(URL);

  const auto is = SharedXmlInputSource::getSharedInputSource(*current_jar.get());
  is->open();

  auto stream = unzip(is->getSrc(), is->getSize(), path_in_jar);

  xmlParserInputBufferPtr buf = xmlParserInputBufferCreateMem(reinterpret_cast<const char*>(stream.data()),
                                                              static_cast<int>(stream.size()), XML_CHAR_ENCODING_NONE);
  xmlParserInputPtr pInput = xmlNewIOInputStream(ctxt, buf, XML_CHAR_ENCODING_NONE);
  pInput->filename = strdup(UStr::to_stdstr(&path_in_jar).c_str());
  return pInput;
}
#endif

xmlParserInputPtr LibXmlReader::xmlMyExternalEntityLoader(const char* URL, const char* /*ID*/,
                                                          const xmlParserCtxtPtr ctxt)
{
  xmlParserInputPtr ret = nullptr;
  // тут обработка имени файла для внешнего entity
  // при этом если в entity указан нормальный путь файловой системы, без всяких переменных окружения, архивов,
  // но можно с комбинацией ./ ../
  //  то в url будет указан полный путь, относительно текущего файла. libxml сама склеит путь.
  //  Иначе в url будет указан путь из самого entity, и дальше с ним над самому разбираться.
  //
  // в ctxt нет информации об обрабатываемом файле.
#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
  if (current_jar) {
    ret = xmlZipEntityLoader(URL, ctxt);
    return ret;
  }
#endif
  ret = xmlNewInputFromFile(ctxt, URL);
  return ret;
}

void LibXmlReader::xml_error_func(void* /*ctx*/, const char* msg, ...)
{
  static char buf[4096];
  static int slen = 0;
  va_list args;

  /* libxml2 prints IO errors from bad includes paths by
   * calling the error function once per word. So we get to
   * re-assemble the message here and print it when we get
   * the line break. My enthusiasm about this is indescribable.
   */
  va_start(args, msg);
  const int rc = vsnprintf(&buf[slen], sizeof(buf) - slen, msg, args);
  va_end(args);

  /* This shouldn't really happen */
  if (rc < 0) {
    COLORER_LOG_ERROR("+++ out of cheese error. redo from start +++\n");
    slen = 0;
    memset(buf, 0, sizeof(buf));
    return;
  }

  slen += rc;
  if (slen >= (int) sizeof(buf)) {
    /* truncated, let's flush this */
    buf[sizeof(buf) - 1] = '\n';
    slen = sizeof(buf);
  }

  /* We're assuming here that the last character is \n. */
  if (buf[slen - 1] == '\n') {
    buf[slen - 1] = '\0';
    COLORER_LOG_ERROR("%", buf);
    memset(buf, 0, sizeof(buf));
    slen = 0;
  }
}