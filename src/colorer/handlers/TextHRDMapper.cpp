#include <colorer/handlers/TextHRDMapper.h>
#include <colorer/parsers/XmlTagDefs.h>
#include <colorer/xml/XmlParserErrorHandler.h>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <memory>

TextHRDMapper::TextHRDMapper() = default;

TextHRDMapper::~TextHRDMapper()
{
  for (auto& regionDefine : regionDefines) {
    const TextRegion* rd = TextRegion::cast(regionDefine.second);
    delete rd->start_text;
    delete rd->end_text;
    delete rd->start_back;
    delete rd->end_back;
  }
}

void TextHRDMapper::loadRegionMappings(XmlInputSource* is)
{
  xercesc::XercesDOMParser xml_parser;
  XmlParserErrorHandler error_handler;
  xml_parser.setErrorHandler(&error_handler);
  xml_parser.setLoadExternalDTD(false);
  xml_parser.setSkipDTDValidation(true);
  xml_parser.parse(*is->getInputSource());
  if (error_handler.getSawErrors()) {
    throw Exception("Error loading HRD file '" + UnicodeString(is->getInputSource()->getSystemId()) + "'");
  }
  xercesc::DOMDocument* hrdbase = xml_parser.getDocument();
  xercesc::DOMElement* hbase = hrdbase->getDocumentElement();

  if (!hbase || !xercesc::XMLString::equals(hbase->getNodeName(), hrdTagHrd)) {
    throw Exception("Incorrect hrd-file structure. Main '<hrd>' block not found. Current file " + UnicodeString(is->getInputSource()->getSystemId()));
  }
  for (xercesc::DOMNode* curel = hbase->getFirstChild(); curel; curel = curel->getNextSibling()) {
    if (curel->getNodeType() == xercesc::DOMNode::ELEMENT_NODE && xercesc::XMLString::equals(curel->getNodeName(), hrdTagAssign)) {
      if (auto* subelem = dynamic_cast<xercesc::DOMElement*>(curel)) {
        const XMLCh* xname = subelem->getAttribute(hrdAssignAttrName);
        if (*xname == '\0') {
          continue;
        }

        auto* name = new UnicodeString(xname);
        auto tp = regionDefines.find(*name);
        if (tp != regionDefines.end()) {
          const TextRegion* rd = TextRegion::cast(tp->second);
          delete rd->start_text;
          delete rd->end_text;
          delete rd->start_back;
          delete rd->end_back;
          regionDefines.erase(tp);
        }
        uUnicodeString stext;
        uUnicodeString etext;
        uUnicodeString sback;
        uUnicodeString eback;
        const XMLCh* sval;
        sval = subelem->getAttribute(hrdAssignAttrSText);
        if (*sval != '\0') {
          stext = std::make_unique<UnicodeString>(sval);
        }
        sval = subelem->getAttribute(hrdAssignAttrEText);
        if (*sval != '\0') {
          etext = std::make_unique<UnicodeString>(sval);
        }
        sval = subelem->getAttribute(hrdAssignAttrSBack);
        if (*sval != '\0') {
          sback = std::make_unique<UnicodeString>(sval);
        }
        sval = subelem->getAttribute(hrdAssignAttrEBack);
        if (*sval != '\0') {
          eback = std::make_unique<UnicodeString>(sval);
        }

        RegionDefine* rdef = new TextRegion(stext.get(), etext.get(), sback.get(), eback.get());
        std::pair<UnicodeString, RegionDefine*> pp(*name, rdef);
        regionDefines.emplace(pp);
        delete name;
      }
    }
  }
}

void TextHRDMapper::saveRegionMappings(Writer* writer) const
{
  writer->write("<?xml version=\"1.0\"?>\n");
  for (const auto& regionDefine : regionDefines) {
    const TextRegion* rdef = TextRegion::cast(regionDefine.second);
    writer->write("  <define name='" + regionDefine.first + "'");
    if (rdef->start_text != nullptr) {
      writer->write(" start_text='" + *rdef->start_text + "'");
    }
    if (rdef->end_text != nullptr) {
      writer->write(" end_text='" + *rdef->end_text + "'");
    }
    if (rdef->start_back != nullptr) {
      writer->write(" start_back='" + *rdef->start_back + "'");
    }
    if (rdef->end_back != nullptr) {
      writer->write(" end_back='" + *rdef->end_back + "'");
    }
    writer->write("/>\n");
  }
  writer->write("\n</hrd>\n");
}

void TextHRDMapper::setRegionDefine(const UnicodeString& name, const RegionDefine* rd)
{
  const TextRegion* rd_new = TextRegion::cast(rd);
  if (!rd_new)
    return;

  uUnicodeString stext;
  uUnicodeString etext;
  uUnicodeString sback;
  uUnicodeString eback;
  if (rd_new->start_text) {
    stext = std::make_unique<UnicodeString>(*rd_new->start_text);
  }
  if (rd_new->end_text) {
    etext = std::make_unique<UnicodeString>(*rd_new->end_text);
  }
  if (rd_new->start_back) {
    sback = std::make_unique<UnicodeString>(*rd_new->start_back);
  }
  if (rd_new->end_back) {
    eback = std::make_unique<UnicodeString>(*rd_new->end_back);
  }

  auto rd_old = regionDefines.find(name);
  if (rd_old != regionDefines.end()) {
    const TextRegion* rdef = TextRegion::cast(rd_old->second);
    delete rdef->start_text;
    delete rdef->end_text;
    delete rdef->start_back;
    delete rdef->end_back;
  }

  RegionDefine* new_region = new TextRegion(stext.get(), etext.get(), sback.get(), eback.get());
  std::pair<UnicodeString, RegionDefine*> p(name, new_region);
  regionDefines.emplace(p);

  // Searches and replaces old region references
  for (auto& idx : regionDefinesCache)
    if (idx == rd_old->second) {
      idx = new_region;
      break;
    }
}
