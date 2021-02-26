#include <colorer/handlers/TextHRDMapper.h>
#include <colorer/parsers/XmlTagDefs.h>
#include <colorer/xml/XmlParserErrorHandler.h>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

TextHRDMapper::~TextHRDMapper()
{
  regionDefines.clear();
}

void TextHRDMapper::loadRegionMappings(XmlInputSource* is)
{
  xercesc::XercesDOMParser xml_parser;
  XmlParserErrorHandler error_handler;

  xml_parser.setErrorHandler(&error_handler);
  xml_parser.setLoadExternalDTD(false);
  xml_parser.setLoadSchema(false);
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
        if (*xname == xercesc::chNull) {
          continue;
        }

        UnicodeString name(xname);
        auto tp = regionDefines.find(name);
        if (tp != regionDefines.end()) {
          spdlog::warn("Duplicate region name '{0}' in file '{1}'. Previous value replaced.", name, UnicodeString(is->getInputSource()->getSystemId()));
          regionDefines.erase(tp);
        }
        std::shared_ptr<const UnicodeString> stext;
        std::shared_ptr<const UnicodeString> etext;
        std::shared_ptr<const UnicodeString> sback;
        std::shared_ptr<const UnicodeString> eback;
        const XMLCh* sval;
        sval = subelem->getAttribute(hrdAssignAttrSText);
        if (*sval != xercesc::chNull) {
          stext = std::make_unique<UnicodeString>(sval);
        }
        sval = subelem->getAttribute(hrdAssignAttrEText);
        if (*sval != xercesc::chNull) {
          etext = std::make_unique<UnicodeString>(sval);
        }
        sval = subelem->getAttribute(hrdAssignAttrSBack);
        if (*sval != xercesc::chNull) {
          sback = std::make_unique<UnicodeString>(sval);
        }
        sval = subelem->getAttribute(hrdAssignAttrEBack);
        if (*sval != xercesc::chNull) {
          eback = std::make_unique<UnicodeString>(sval);
        }

        auto rdef = std::unique_ptr<RegionDefine>(new TextRegion(stext, etext, sback, eback));
        regionDefines.emplace(name, std::move(rdef));
      }
    }
  }
}

void TextHRDMapper::saveRegionMappings(Writer* writer) const
{
  writer->write("<?xml version=\"1.0\"?>\n");
  for (const auto& regionDefine : regionDefines) {
    const TextRegion* rdef = TextRegion::cast(regionDefine.second.get());
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
  if (!rd)
    return;

  const TextRegion* rd_new = TextRegion::cast(rd);
  RegionDefine* new_region = new TextRegion(*rd_new);

  auto rd_old_it = regionDefines.find(name);
  if (rd_old_it == regionDefines.end()) {
    std::pair<UnicodeString, RegionDefine*> pp(name, new_region);
    regionDefines.emplace(pp);
  } else {
    rd_old_it->second.reset(new_region);
  }
}
