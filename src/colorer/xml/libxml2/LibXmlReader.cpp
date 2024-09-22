#include "colorer/xml/libxml2/LibXmlReader.h"
#include <libxml/parserInternals.h>
#include <cstring>
#include <fstream>
#include "colorer/Exception.h"
#include "colorer/utils/Environment.h"

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
#include "colorer/zip/MemoryFile.h"
#endif

#ifdef _MSC_VER
#define strdup(p) _strdup(p)
#endif

uUnicodeString LibXmlReader::current_file = nullptr;
bool LibXmlReader::is_full_path = false;

LibXmlReader::LibXmlReader(const UnicodeString& source_file) : xmldoc(nullptr)
{
  xmlSetExternalEntityLoader(xmlMyExternalEntityLoader);
  xmlSetGenericErrorFunc(nullptr, xml_error_func);

  current_file = std::make_unique<UnicodeString>(source_file);
  is_full_path = true;

  xmldoc = xmlReadFile(UStr::to_stdstr(&source_file).c_str(), nullptr, XML_PARSE_NOENT | XML_PARSE_NONET);
}

LibXmlReader::LibXmlReader(const XmlInputSource& source) : LibXmlReader(source.getPath()) {}

LibXmlReader::~LibXmlReader()
{
  if (xmldoc != nullptr) {
    xmlFreeDoc(xmldoc);
  }
  current_file.reset();
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
    result.name = UnicodeString(reinterpret_cast<const char*>(node->name));

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
      return UnicodeString(reinterpret_cast<const char*>(child->content));
    }
    if (child->type == XML_TEXT_NODE) {
      auto temp_string = UnicodeString(reinterpret_cast<const char*>(child->content));
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
    data.emplace(std::pair(reinterpret_cast<const char*>(attr->name), reinterpret_cast<const char*>(content)));
    xmlFree(content);
  }
}

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
xmlParserInputPtr LibXmlReader::xmlZipEntityLoader(const PathInJar& paths, const xmlParserCtxtPtr ctxt)
{
  const auto is = SharedXmlInputSource::getSharedInputSource(paths.path_to_jar);
  is->open();

  const auto unzipped_stream = unzip(is->getSrc(), is->getSize(), paths.path_in_jar);

  xmlParserInputBufferPtr buf =
      xmlParserInputBufferCreateMem(reinterpret_cast<const char*>(unzipped_stream->data()),
                                    static_cast<int>(unzipped_stream->size()), XML_CHAR_ENCODING_NONE);
  xmlParserInputPtr pInput = xmlNewIOInputStream(ctxt, buf, XML_CHAR_ENCODING_NONE);

  // заполняем filename для работы external entity
  auto root_pos = paths.path_in_jar.lastIndexOf('/') + 1;
  auto file_name = UnicodeString(paths.path_in_jar, root_pos);
  pInput->filename = strdup(UStr::to_stdstr(&file_name).c_str());
  return pInput;
}
#endif

xmlParserInputPtr LibXmlReader::xmlMyExternalEntityLoader(const char* URL, const char* /*ID*/,
                                                          const xmlParserCtxtPtr ctxt)
{
  // Функция вызывается перед каждым открытием файла в рамках libxml
  //  будь то xmlReadFile, или открытие файла для external entity.
  // Т.е. путь к файлу можно проверять или модифицировать тут.
  // Если в external entity указан путь похожий на файловый, то libxml сам формирует путь до entity файла,
  //  путем склейки пути от текущего файла и указанного в external entity.
  // При этом в параметрах функции или в контексте нет ни пути до исходного файла, ни до файла в entity

  const UnicodeString string_url(URL);

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
  if (string_url.startsWith(jar) || current_file->startsWith(jar)) {
    auto paths = LibXmlInputSource::getFullPathFromPathJar(string_url, is_full_path ? nullptr : current_file.get());
    is_full_path = false;
    xmlParserInputPtr ret = nullptr;
    try {
      ret = xmlZipEntityLoader(paths, ctxt);
    } catch (...) {
    }
    return ret;
  }
#endif
  is_full_path = false;
  xmlParserInputPtr ret = xmlNewInputFromFile(ctxt, URL);

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
  if (slen >= static_cast<int>(sizeof(buf))) {
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