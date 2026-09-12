#include "colorer/xml/libxml2/LibXmlReader.h"
#include <libxml/parserInternals.h>
#include <libxml/uri.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <utility>
#include "colorer/Exception.h"
#include "colorer/base/BaseNames.h"
#include "colorer/utils/Environment.h"

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
#include "colorer/zip/MemoryFile.h"
#endif

#ifdef _MSC_VER
#define strdup(p) _strdup(p)
#endif

namespace {

struct XmlLoadContext
{
  UnicodeString current_file;
  bool is_first_call = true;
};

XmlLoadContext* current_load = nullptr;

class XmlLoadCurrent
{
 public:
  explicit XmlLoadCurrent(XmlLoadContext* load) : previous(current_load)
  {
    current_load = load;
  }
  ~XmlLoadCurrent()
  {
    current_load = previous;
  }
  XmlLoadCurrent(const XmlLoadCurrent&) = delete;
  XmlLoadCurrent& operator=(const XmlLoadCurrent&) = delete;

 private:
  XmlLoadContext* previous;
};

XmlLoadContext* loadContext(xmlParserCtxtPtr ctxt)
{
  if (current_load != nullptr) {
    return current_load;
  }
  if (ctxt != nullptr && ctxt->_private != nullptr) {
    return static_cast<XmlLoadContext*>(ctxt->_private);
  }
  return nullptr;
}

UnicodeString decodeFilesystemXmlUrl(const UnicodeString& url)
{
  const auto utf8 = colorer::Environment::to_utf8_path(url);
  char* const unescaped = xmlURIUnescapeString(utf8.c_str(), -1, nullptr);
  UnicodeString decoded;
  if (unescaped != nullptr) {
    decoded = *Encodings::fromUTF8(unescaped, static_cast<int32_t>(strlen(unescaped)));
    xmlFree(unescaped);
  }
  else {
    decoded = url;
  }

  static const UnicodeString file_localhost(u"file://localhost");
  static const UnicodeString file_slashes(u"file://");
  static const UnicodeString file_scheme(u"file:");
  if (decoded.startsWith(file_localhost)) {
    decoded = UnicodeString(decoded, file_localhost.length());
  }
  else if (decoded.startsWith(file_slashes)) {
    decoded = UnicodeString(decoded, file_slashes.length());
  }
  else if (decoded.startsWith(file_scheme)) {
    decoded = UnicodeString(decoded, file_scheme.length());
  }

#ifdef WIN32
  if (decoded.startsWith("/") && decoded.length() > 2 && decoded[2] == ':') {
    decoded = UnicodeString(decoded, 1);
  }
#endif
  return decoded;
}

bool isRemoteHttpUrl(const UnicodeString& url)
{
  return url.startsWith(u"http://") || url.startsWith(u"https://");
}

char xml_err_buf[4096];
int xml_err_slen = 0;

}  // namespace

LibXmlReader::LibXmlReader(const UnicodeString& source_file)
{
  xmlSetExternalEntityLoader(xmlMyExternalEntityLoader);
  xmlSetGenericErrorFunc(nullptr, xml_error_func);

  xmlParserCtxtPtr ctxt = xmlNewParserCtxt();
  if (ctxt == nullptr) {
    return;
  }

  XmlLoadContext load;
  load.current_file = source_file;
  ctxt->_private = &load;
  const XmlLoadCurrent current(&load);

  xmlDocPtr xmldoc =
      xmlCtxtReadFile(ctxt, colorer::Environment::to_utf8_path(source_file).c_str(), nullptr, XML_PARSE_NOENT | XML_PARSE_NONET);
  parsed = xmldoc != nullptr;
  if (xmldoc != nullptr) {
    xmlNode* current_node = xmlDocGetRootElement(xmldoc);
    size_t count = 0;
    for (xmlNode* node = current_node; node != nullptr; node = node->next) {
      ++count;
    }
    nodes.reserve(count);
    while (current_node != nullptr) {
      XMLNode result;
      populateNode(current_node, result);
      nodes.push_back(std::move(result));
      current_node = current_node->next;
    }
    xmlFreeDoc(xmldoc);
    ctxt->myDoc = nullptr;
  }
  ctxt->_private = nullptr;
  xmlFreeParserCtxt(ctxt);
}

LibXmlReader::LibXmlReader(const XmlInputSource& source) : LibXmlReader(source.getPath()) {}

LibXmlReader::~LibXmlReader() = default;

void LibXmlReader::parse(XMLNodeList& out_nodes)
{
  out_nodes = std::move(nodes);
}

bool LibXmlReader::populateNode(xmlNode* node, XMLNode& result)
{
  if (node->type == XML_ELEMENT_NODE) {
    result.name = UnicodeString(reinterpret_cast<const char*>(node->name));

    const auto text_string = getElementText(node);
    if (text_string && !text_string->isEmpty()) {
      result.text = UnicodeString(*text_string.get());
    }
    getChildren(node, result);
    getAttributes(node, result.attributes);

    return true;
  }
  return false;
}

uUnicodeString LibXmlReader::getElementText(const xmlNode* node)
{
  for (const xmlNode* child = node->children; child != nullptr; child = child->next) {
    if (child->type == XML_CDATA_SECTION_NODE) {
      return Encodings::fromUTF8(child->content);
    }
    if (child->type == XML_TEXT_NODE) {
      auto temp_string = Encodings::fromUTF8(child->content);
      temp_string->trim();
      if (temp_string->isEmpty()) {
        continue;
      }
      return temp_string;
    }
  }
  return nullptr;
}

void LibXmlReader::getChildren(xmlNode* node, XMLNode& result)
{
  if (node->children == nullptr) {
    return;
  }

  size_t count = 0;
  for (const xmlNode* child = node->children; child != nullptr; child = child->next) {
    if (child->type == XML_ELEMENT_NODE) {
      ++count;
    }
  }
  result.children.reserve(count);

  for (xmlNode* child = node->children; child != nullptr; child = child->next) {
    if (child->type != XML_ELEMENT_NODE) {
      continue;
    }
    XMLNode xml_child;
    if (populateNode(child, xml_child)) {
      result.children.push_back(std::move(xml_child));
    }
  }
}

void LibXmlReader::getAttributes(const xmlNode* node, std::unordered_map<UnicodeString, UnicodeString>& data)
{
  for (xmlAttrPtr attr = node->properties; attr != nullptr; attr = attr->next) {
    const auto content = xmlNodeGetContent(attr->children);
    auto decoded_string = Encodings::fromUTF8(content);
    data.try_emplace(reinterpret_cast<const char*>(attr->name), *decoded_string.get());
    xmlFree(content);
  }
}

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
xmlParserInputPtr LibXmlReader::xmlZipEntityLoader(const PathInJar& paths, xmlParserCtxtPtr ctxt)
{
  const auto is = SharedXmlInputSource::getSharedInputSource(paths.path_to_jar);
  std::unique_ptr<std::vector<byte>> unzipped_stream;
  try {
    is->open();
    unzipped_stream = unzip(is->getSrc(), is->getSize(), paths.path_in_jar);
  } catch (...) {
    is->delref();
    throw;
  }
  is->delref();

  xmlParserInputBufferPtr buf =
      xmlParserInputBufferCreateMem(reinterpret_cast<const char*>(unzipped_stream->data()),
                                    static_cast<int>(unzipped_stream->size()), XML_CHAR_ENCODING_NONE);
  xmlParserInputPtr pInput = xmlNewIOInputStream(ctxt, buf, XML_CHAR_ENCODING_NONE);

  // filling in the filename for the external entity to work
  const auto root_pos = paths.path_in_jar.lastIndexOf('/') + 1;
  const auto file_name = UnicodeString(paths.path_in_jar, root_pos);
  pInput->filename = strdup(UStr::to_stdstr(&file_name).c_str());
  return pInput;
}
#endif

xmlParserInputPtr LibXmlReader::xmlMyExternalEntityLoader(const char* URL, const char* /*ID*/, xmlParserCtxtPtr ctxt)
{
  /*
   * Called for the main xmlCtxtReadFile document and for every external entity.
   * libxml2 joins a SYSTEM path to the current document URI, but that fails when the
   * base path has a space (xmlBuildURI) or, on Linux, non-Latin letters. The URL then
   * stays relative (hrd/foo.xml). Resolve it against current_file instead of probing
   * the process cwd, which can pick up a different catalog.
   */

  XmlLoadContext* load = loadContext(ctxt);
  if (load == nullptr) {
    return xmlNewInputFromFile(ctxt, URL);
  }
  if (URL == nullptr) {
    return nullptr;
  }

  auto filename = Encodings::fromUTF8(const_cast<char*>(URL), static_cast<int32_t>(strlen(URL)));
  UnicodeString string_url(*filename.get());

  // read entity string like "env:$FAR_HOME/hrd/catalog-console.xml"
  static const UnicodeString env(u"env:");
  if (!load->is_first_call && string_url.startsWith(env)) {
    const auto exp = colorer::Environment::expandSpecialEnvironment(string_url);
    string_url = UnicodeString(exp, env.length());
  }

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
  if (string_url.startsWith(jar) || load->current_file.startsWith(jar)) {
    const auto paths =
        LibXmlInputSource::getFullPathsToZip(string_url, load->is_first_call ? nullptr : &load->current_file);
    load->is_first_call = false;
    xmlParserInputPtr ret = nullptr;
    try {
      ret = xmlZipEntityLoader(paths, ctxt);
    } catch (const Exception& e) {
      // Must not throw through libxml2's C entity-loader callback.
      COLORER_LOG_ERROR("zip entity load failed: %", e.what());
    } catch (...) {
      COLORER_LOG_ERROR("zip entity load failed");
    }
    return ret;
  }
#endif

  if (isRemoteHttpUrl(string_url)) {
    load->is_first_call = false;
    return nullptr;
  }

  string_url = decodeFilesystemXmlUrl(string_url);

  if (!load->is_first_call) {
    const auto as_path = colorer::Environment::to_filepath(&string_url);
    if (!as_path.is_absolute() || !colorer::Environment::isRegularFile(string_url)) {
      auto resolved = colorer::Environment::getAbsolutePath(load->current_file, string_url);
      if (colorer::Environment::isRegularFile(resolved)) {
        string_url = std::move(resolved);
      }
    }
  }

  load->is_first_call = false;
  return xmlNewInputFromFile(ctxt, colorer::Environment::to_utf8_path(string_url).c_str());
}

void LibXmlReader::xml_error_func(void* /*ctx*/, const char* msg, ...)
{
  va_list args;

  /* libxml2 prints IO errors from bad includes paths by
   * calling the error function once per word. So we get to
   * re-assemble the message here and print it when we get
   * the line break. My enthusiasm about this is indescribable.
   */
  va_start(args, msg);
  const int rc = vsnprintf(&xml_err_buf[xml_err_slen], sizeof(xml_err_buf) - xml_err_slen, msg, args);
  va_end(args);

  /* This shouldn't really happen */
  if (rc < 0) {
    COLORER_LOG_ERROR("+++ out of cheese error. redo from start +++\n");
    xml_err_slen = 0;
    memset(xml_err_buf, 0, sizeof(xml_err_buf));
    return;
  }

  xml_err_slen += rc;
  if (xml_err_slen >= static_cast<int>(sizeof(xml_err_buf))) {
    /* truncated, let's flush this */
    xml_err_buf[sizeof(xml_err_buf) - 1] = '\n';
    xml_err_slen = sizeof(xml_err_buf);
  }

  /* We're assuming here that the last character is \n. */
  if (xml_err_buf[xml_err_slen - 1] == '\n') {
    xml_err_buf[xml_err_slen - 1] = '\0';
    COLORER_LOG_ERROR("%", xml_err_buf);
    memset(xml_err_buf, 0, sizeof(xml_err_buf));
    xml_err_slen = 0;
  }
}
