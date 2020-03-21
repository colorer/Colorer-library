#include <colorer/common/UStr.h>
#include <colorer/handlers/StyledHRDMapper.h>
#include <colorer/parsers/XmlTagDefs.h>
#include <colorer/xml/XmlParserErrorHandler.h>
#include <cstdio>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

const int StyledRegion::RD_BOLD = 1;
const int StyledRegion::RD_ITALIC = 2;
const int StyledRegion::RD_UNDERLINE = 4;
const int StyledRegion::RD_STRIKEOUT = 8;

StyledHRDMapper::StyledHRDMapper() = default;

StyledHRDMapper::~StyledHRDMapper()
{
  for (const auto& it : regionDefines) {
    delete it.second;
  }
  regionDefines.clear();
}

void StyledHRDMapper::loadRegionMappings(XmlInputSource* is)
{
  xercesc::XercesDOMParser xml_parser;
  XmlParserErrorHandler error_handler;
  xml_parser.setErrorHandler(&error_handler);
  xml_parser.setLoadExternalDTD(false);
  xml_parser.setSkipDTDValidation(true);
  xml_parser.parse(*is->getInputSource());
  if (error_handler.getSawErrors()) {
    throw Exception("Error loading HRD file");
  }
  xercesc::DOMDocument* hrdbase = xml_parser.getDocument();
  xercesc::DOMElement* hbase = hrdbase->getDocumentElement();

  if (hbase == nullptr || !xercesc::XMLString::equals(hbase->getNodeName(), hrdTagHrd)) {
    throw Exception("Error loading HRD file");
  }

  for (xercesc::DOMNode* curel = hbase->getFirstChild(); curel; curel = curel->getNextSibling()) {
    if (curel->getNodeType() == xercesc::DOMNode::ELEMENT_NODE && xercesc::XMLString::equals(curel->getNodeName(), hrdTagAssign)) {
      if (auto* subelem = dynamic_cast<xercesc::DOMElement*>(curel)) {
        const XMLCh* xname = subelem->getAttribute(hrdAssignAttrName);
        if (*xname == '\0') {
          continue;
        }

        auto* name = new UnicodeString(xname);
        auto rd_new = regionDefines.find(*name);
        if (rd_new != regionDefines.end()) {
          regionDefines.erase(rd_new);
        }

        int val = 0;
        UnicodeString dhrdAssignAttrFore = UnicodeString(subelem->getAttribute(hrdAssignAttrFore));
        bool bfore = UStr::getNumber(&dhrdAssignAttrFore, &val);
        int fore = val;
        UnicodeString dhrdAssignAttrBack = UnicodeString(subelem->getAttribute(hrdAssignAttrBack));
        bool bback = UStr::getNumber(&dhrdAssignAttrBack, &val);
        int back = val;
        int style = 0;
        UnicodeString dhrdAssignAttrStyle = UnicodeString(subelem->getAttribute(hrdAssignAttrStyle));
        if (UStr::getNumber(&dhrdAssignAttrStyle, &val)) {
          style = val;
        }

        RegionDefine* rdef = new StyledRegion(bfore, bback, fore, back, style);
        std::pair<UnicodeString, RegionDefine*> pp(*name, rdef);
        regionDefines.emplace(pp);

        delete name;
      }
    }
  }
}

/** Writes all currently loaded region definitions into
    XML file. Note, that this method writes all loaded
    defines from all loaded HRD files.
*/
void StyledHRDMapper::saveRegionMappings(Writer* writer) const
{
  writer->write("<?xml version=\"1.0\"?>\n<!DOCTYPE hrd SYSTEM \"../hrd.dtd\">\n\n<hrd>\n");
  for (const auto& regionDefine : regionDefines) {
    const StyledRegion* rdef = StyledRegion::cast(regionDefine.second);
    char temporary[256];
    writer->write("  <define name='" + regionDefine.first + "'");
    if (rdef->bfore) {
      sprintf(temporary, " fore=\"#%06x\"", rdef->fore);
      writer->write(temporary);
    }
    if (rdef->bback) {
      sprintf(temporary, " back=\"#%06x\"", rdef->back);
      writer->write(temporary);
    }
    if (rdef->style) {
      sprintf(temporary, " style=\"#%06x\"", rdef->style);
      writer->write(temporary);
    }
    writer->write("/>\n");
  }
  writer->write("\n</hrd>\n");
}

/** Adds or replaces region definition */
void StyledHRDMapper::setRegionDefine(const UnicodeString& name, const RegionDefine* rd)
{
  auto rd_old = regionDefines.find(name);

  const StyledRegion* new_region = StyledRegion::cast(rd);
  RegionDefine* rd_new = new StyledRegion(*new_region);
  std::pair<UnicodeString, RegionDefine*> pp(name, rd_new);
  regionDefines.emplace(pp);

  // Searches and replaces old region references
  for (auto& idx : regionDefinesVector) {
    if (idx == rd_old->second) {
      idx = rd_new;
      break;
    }
  }
}
