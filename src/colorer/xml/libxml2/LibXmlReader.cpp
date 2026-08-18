#include "colorer/xml/libxml2/LibXmlReader.h"
#include <libxml/parserInternals.h>
#include <cstdarg>
#include <cstring>
#include <memory>
#include <unordered_map>
#include "colorer/Exception.h"
#include "colorer/base/BaseNames.h"
#include "colorer/utils/Environment.h"
#include "colorer/xml/XmlLoadSession.h"

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
#include "colorer/xml/libxml2/SharedXmlInputSource.h"
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
#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
  std::unordered_map<UnicodeString, std::unique_ptr<SharedXmlInputSource>> jars;
#endif
};

XmlLoadContext* loadContext(xmlParserCtxtPtr ctxt)
{
  if (ctxt == nullptr) {
    return nullptr;
  }
  return static_cast<XmlLoadContext*>(ctxt->_private);
}

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
thread_local XmlJarCache* g_active_jars = nullptr;
#endif

}  // namespace

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
XmlJarCache* XmlLoadSession::active()
{
  return g_active_jars;
}
#endif

XmlLoadSession::XmlLoadSession(XmlJarCache& cache)
{
#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
  prev = g_active_jars;
  g_active_jars = &cache;
#else
  (void) cache;
#endif
}

XmlLoadSession::~XmlLoadSession()
{
#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
  g_active_jars = prev;
#endif
}

LibXmlReader::LibXmlReader(const UnicodeString& source_file)
{
  installLibXmlHooks();

  XmlLoadContext load_ctx;
  load_ctx.current_file = source_file;

  xmlParserCtxtPtr ctxt = xmlNewParserCtxt();
  if (ctxt == nullptr) {
    return;
  }
  ctxt->_private = &load_ctx;
  xmldoc = xmlCtxtReadFile(ctxt, UStr::to_stdstr(&source_file).c_str(), nullptr,
                           XML_PARSE_NOENT | XML_PARSE_NONET);
  ctxt->_private = nullptr;
  xmlFreeParserCtxt(ctxt);
}

LibXmlReader::LibXmlReader(const XmlInputSource& source) : LibXmlReader(source.getPath()) {}

LibXmlReader::~LibXmlReader()
{
  if (xmldoc != nullptr) {
    xmlFreeDoc(xmldoc);
  }
}

void LibXmlReader::installLibXmlHooks()
{
  xmlSetExternalEntityLoader(xmlMyExternalEntityLoader);
  xmlSetGenericErrorFunc(nullptr, xml_error_func);
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
    const auto content = xmlNodeGetContent(attr->children);
    auto decoded_string = Encodings::fromUTF8(content);
    data.try_emplace(reinterpret_cast<const char*>(attr->name), *decoded_string.get());
    xmlFree(content);
  }
}

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
xmlParserInputPtr LibXmlReader::xmlZipEntityLoader(const PathInJar& paths, xmlParserCtxtPtr ctxt)
{
  SharedXmlInputSource* is = nullptr;
  std::unique_ptr<SharedXmlInputSource> owned;
  auto* session_jars = XmlLoadSession::active();
  if (session_jars != nullptr) {
    auto& slot = session_jars->jars[paths.path_to_jar];
    if (!slot) {
      slot.reset(SharedXmlInputSource::getSharedInputSource(paths.path_to_jar));
      slot->open();
    }
    is = slot.get();
  }
  else {
    auto* ctx = loadContext(ctxt);
    if (ctx != nullptr) {
      auto& slot = ctx->jars[paths.path_to_jar];
      if (!slot) {
        slot.reset(SharedXmlInputSource::getSharedInputSource(paths.path_to_jar));
        slot->open();
      }
      is = slot.get();
    }
    else {
      owned.reset(SharedXmlInputSource::getSharedInputSource(paths.path_to_jar));
      owned->open();
      is = owned.get();
    }
  }

  const auto unzipped_stream = unzip(is->getSrc(), is->getSize(), paths.path_in_jar);

  xmlParserInputBufferPtr buf =
      xmlParserInputBufferCreateMem(reinterpret_cast<const char*>(unzipped_stream->data()),
                                    static_cast<int>(unzipped_stream->size()), XML_CHAR_ENCODING_NONE);
  xmlParserInputPtr pInput = xmlNewIOInputStream(ctxt, buf, XML_CHAR_ENCODING_NONE);

  const auto root_pos = paths.path_in_jar.lastIndexOf('/') + 1;
  const auto file_name = UnicodeString(paths.path_in_jar, root_pos);
  pInput->filename = strdup(UStr::to_stdstr(&file_name).c_str());
  return pInput;
}
#endif

xmlParserInputPtr LibXmlReader::xmlMyExternalEntityLoader(const char* URL, const char* /*ID*/, xmlParserCtxtPtr ctxt)
{
  /*
   * Called before each file open inside libxml (the main xmlCtxtReadFile
   * and every external entity). Relative entity paths are resolved against
   * the document being parsed — that path is stored on ctxt->_private so a
   * second ParserFactory can load XML without clobbering another in-flight
   * parse's current file.
   */
  auto* ctx = loadContext(ctxt);

  auto filename = Encodings::fromUTF8(const_cast<char*>(URL), static_cast<int32_t>(strlen(URL)));
  UnicodeString string_url(*filename.get());

  const bool first_call = ctx == nullptr || ctx->is_first_call;
  const UnicodeString* current_file = ctx != nullptr ? &ctx->current_file : nullptr;

  static const UnicodeString env(u"env:");
  if (!first_call && string_url.startsWith(env)) {
    const auto exp = colorer::Environment::expandSpecialEnvironment(string_url);
    string_url = UnicodeString(exp, env.length());
  }

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
  if (string_url.startsWith(jar) || (current_file != nullptr && current_file->startsWith(jar))) {
    const auto paths = LibXmlInputSource::getFullPathsToZip(string_url, first_call ? nullptr : current_file);
    if (ctx != nullptr) {
      ctx->is_first_call = false;
    }
    xmlParserInputPtr ret = nullptr;
    try {
      ret = xmlZipEntityLoader(paths, ctxt);
    } catch (...) {
    }
    return ret;
  }
#endif
  if (!first_call && current_file != nullptr && !colorer::Environment::isRegularFile(string_url)) {
    auto new_string_url = colorer::Environment::getAbsolutePath(*current_file, string_url);
    if (colorer::Environment::isRegularFile(new_string_url)) {
      string_url = std::move(new_string_url);
    }
  }

  if (ctx != nullptr) {
    ctx->is_first_call = false;
  }
  xmlParserInputPtr ret = xmlNewInputFromFile(ctxt, UStr::to_stdstr(&string_url).c_str());

  return ret;
}

void LibXmlReader::xml_error_func(void* /*ctx*/, const char* msg, ...)
{
  thread_local char buf[4096];
  thread_local int slen = 0;
  va_list args;

  /* libxml2 prints IO errors from bad includes paths by
   * calling the error function once per word. So we get to
   * re-assemble the message here and print it when we get
   * the line break. My enthusiasm about this is indescribable.
   */
  va_start(args, msg);
  const int rc = vsnprintf(&buf[slen], sizeof(buf) - slen, msg, args);
  va_end(args);

  if (rc < 0) {
    COLORER_LOG_ERROR("+++ out of cheese error. redo from start +++\n");
    slen = 0;
    memset(buf, 0, sizeof(buf));
    return;
  }

  slen += rc;
  if (slen >= static_cast<int>(sizeof(buf))) {
    buf[sizeof(buf) - 1] = '\n';
    slen = sizeof(buf);
  }

  if (buf[slen - 1] == '\n') {
    buf[slen - 1] = '\0';
    COLORER_LOG_ERROR("%", buf);
    memset(buf, 0, sizeof(buf));
    slen = 0;
  }
}
