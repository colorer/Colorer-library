#include <stdio.h>
#include <colorer/handlers/StyledHRDMapper.h>
#include <colorer/unicode/UnicodeTools.h>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <colorer/xml/XmlParserErrorHandler.h>
#include <colorer/xml/XmlTagDefs.h>

const int StyledRegion::RD_BOLD = 1;
const int StyledRegion::RD_ITALIC = 2;
const int StyledRegion::RD_UNDERLINE = 4;
const int StyledRegion::RD_STRIKEOUT = 8;

StyledHRDMapper::StyledHRDMapper() {}
StyledHRDMapper::~StyledHRDMapper()
{
  for (auto it : regionDefines) {
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
    throw Exception(CString("Error loading HRD file"));
  }
  xercesc::DOMDocument* hrdbase = xml_parser.getDocument();
  xercesc::DOMElement* hbase = hrdbase->getDocumentElement();

  if (hbase == nullptr || !xercesc::XMLString::equals(hbase->getNodeName(), hrdTagHrd)) {
    throw Exception(CString("Error loading HRD file"));
  }

  for (xercesc::DOMNode* curel = hbase->getFirstChild(); curel; curel = curel->getNextSibling()) {
    if (curel->getNodeType() == xercesc::DOMNode::ELEMENT_NODE && xercesc::XMLString::equals(curel->getNodeName(), hrdTagAssign)) {
      xercesc::DOMElement* subelem = static_cast<xercesc::DOMElement*>(curel);
      const XMLCh* xname = subelem->getAttribute(hrdAssignAttrName);
      if (*xname == '\0') {
        continue;
      }

      const String* name = new CString(xname);
      auto rd_new = regionDefines.find(name);
      if (rd_new != regionDefines.end()) {
        regionDefines.erase(rd_new);
      }

      int val = 0;
      CString dhrdAssignAttrFore = CString(subelem->getAttribute(hrdAssignAttrFore));
      bool bfore = UnicodeTools::getNumber(&dhrdAssignAttrFore, &val);
      int fore = val;
      CString dhrdAssignAttrBack = CString(subelem->getAttribute(hrdAssignAttrBack));
      bool bback = UnicodeTools::getNumber(&dhrdAssignAttrBack, &val);
      int back = val;
      int style = 0;
      CString dhrdAssignAttrStyle = CString(subelem->getAttribute(hrdAssignAttrStyle));
      if (UnicodeTools::getNumber(&dhrdAssignAttrStyle, &val)) {
        style = val;
      }
      RegionDefine* rdef = new StyledRegion(bfore, bback, fore, back, style);
      std::pair<SString, RegionDefine*> pp(name, rdef);
      regionDefines.emplace(pp);

      delete name;
    }
  }
}

/** Writes all currently loaded region definitions into
    XML file. Note, that this method writes all loaded
    defines from all loaded HRD files.
*/
void StyledHRDMapper::saveRegionMappings(Writer* writer) const
{
  writer->write(CString("<?xml version=\"1.0\"?>\n\
<!DOCTYPE hrd SYSTEM \"../hrd.dtd\">\n\n\
<hrd>\n"));
  for (auto it = regionDefines.begin(); it != regionDefines.end(); ++it) {
    const StyledRegion* rdef = StyledRegion::cast(it->second);
    char temporary[256];
    writer->write(SString("  <define name='") + it->first + "'");
    if (rdef->bfore) {
      sprintf(temporary, " fore=\"#%06x\"", rdef->fore);
      writer->write(CString(temporary));
    }
    if (rdef->bback) {
      sprintf(temporary, " back=\"#%06x\"", rdef->back);
      writer->write(CString(temporary));
    }
    if (rdef->style) {
      sprintf(temporary, " style=\"#%06x\"", rdef->style);
      writer->write(CString(temporary));
    }
    writer->write(CString("/>\n"));
  }
  writer->write(CString("\n</hrd>\n"));
}

/** Adds or replaces region definition */
void StyledHRDMapper::setRegionDefine(const String &name, const RegionDefine* rd)
{
  auto rd_old = regionDefines.find(&name);

  const StyledRegion* new_region = StyledRegion::cast(rd);
  RegionDefine* rd_new = new StyledRegion(*new_region);
  std::pair<SString, RegionDefine*> pp(name, rd_new);
  regionDefines.emplace(pp);

  // Searches and replaces old region references
  for (size_t idx = 0; idx < regionDefinesVector.size(); idx++) {
    if (regionDefinesVector.at(idx) == rd_old->second) {
      regionDefinesVector.at(idx) = rd_new;
      break;
    }
  }
}



