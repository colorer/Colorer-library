#ifndef COLORER_XMLLOADSESSION_H
#define COLORER_XMLLOADSESSION_H

#include "colorer/Common.h"

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
#include <memory>
#include <unordered_map>
#include "colorer/xml/libxml2/SharedXmlInputSource.h"
#endif

/** Per-HrcLibrary zip bytes. Not process-global: a probe ParserFactory
 * re-reads disk instead of seeing the live instance's already-loaded jar.
 */
class XmlJarCache
{
#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
 public:
  std::unordered_map<UnicodeString, std::unique_ptr<SharedXmlInputSource>> jars;
#endif
};

/** Activates cache for XML parses on this thread until destroyed.
 * Nested sessions restore the previous cache pointer.
 */
class XmlLoadSession
{
 public:
  explicit XmlLoadSession(XmlJarCache& cache);
  ~XmlLoadSession();

  XmlLoadSession(const XmlLoadSession&) = delete;
  XmlLoadSession& operator=(const XmlLoadSession&) = delete;

#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
  static XmlJarCache* active();
#endif

 private:
#ifdef COLORER_FEATURE_ZIPINPUTSOURCE
  XmlJarCache* prev {nullptr};
#endif
};

#endif  // COLORER_XMLLOADSESSION_H
