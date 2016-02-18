#include <stdio.h>
#include <colorer/handlers/StyledHRDMapper.h>
#include <unicode/UnicodeTools.h>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xml/XmlParserErrorHandler.h>
#include <xml/XmlTagDefs.h>

const int StyledRegion::RD_BOLD = 1;
const int StyledRegion::RD_ITALIC = 2;
const int StyledRegion::RD_UNDERLINE = 4;
const int StyledRegion::RD_STRIKEOUT = 8;

StyledHRDMapper::StyledHRDMapper() {}
StyledHRDMapper::~StyledHRDMapper()
{
  for(auto it: regionDefines){
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
    throw Exception(DString("Error loading HRD file"));
  }
  xercesc::DOMDocument* hrdbase = xml_parser.getDocument();
  xercesc::DOMElement* hbase = hrdbase->getDocumentElement();

  if (hbase == nullptr || !xercesc::XMLString::equals(hbase->getNodeName(), hrdTagHrd)) {
    throw Exception(DString("Error loading HRD file"));
  }

  for (xercesc::DOMNode* curel = hbase->getFirstChild(); curel; curel = curel->getNextSibling()) {
    if (curel->getNodeType() == xercesc::DOMNode::ELEMENT_NODE && xercesc::XMLString::equals(curel->getNodeName(), hrdTagAssign)) {
      xercesc::DOMElement* subelem = static_cast<xercesc::DOMElement*>(curel);
      const XMLCh* xname = subelem->getAttribute(hrdAssignAttrName);
      if (*xname == '\0') {
        continue;
      }

      const String* name = new DString(xname);
      auto rd_new = regionDefines.find(name);
      if (rd_new != regionDefines.end()) {
        regionDefines.erase(rd_new);
      }

      int val = 0;
      DString dhrdAssignAttrFore = DString(subelem->getAttribute(hrdAssignAttrFore));
      bool bfore = UnicodeTools::getNumber(&dhrdAssignAttrFore, &val);
      int fore = val;
      DString dhrdAssignAttrBack = DString(subelem->getAttribute(hrdAssignAttrBack));
      bool bback = UnicodeTools::getNumber(&dhrdAssignAttrBack, &val);
      int back = val;
      int style = 0;
      DString dhrdAssignAttrStyle = DString(subelem->getAttribute(hrdAssignAttrStyle));
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
  writer->write(DString("<?xml version=\"1.0\"?>\n\
<!DOCTYPE hrd SYSTEM \"../hrd.dtd\">\n\n\
<hrd>\n"));
  for (auto it = regionDefines.begin(); it != regionDefines.end(); ++it) {
    const StyledRegion* rdef = StyledRegion::cast(it->second);
    char temporary[256];
    writer->write(StringBuffer("  <define name='") + it->first + "'");
    if (rdef->bfore) {
      sprintf(temporary, " fore=\"#%06x\"", rdef->fore);
      writer->write(DString(temporary));
    }
    if (rdef->bback) {
      sprintf(temporary, " back=\"#%06x\"", rdef->back);
      writer->write(DString(temporary));
    }
    if (rdef->style) {
      sprintf(temporary, " style=\"#%06x\"", rdef->style);
      writer->write(DString(temporary));
    }
    writer->write(DString("/>\n"));
  }
  writer->write(DString("\n</hrd>\n"));
}

/** Adds or replaces region definition */
void StyledHRDMapper::setRegionDefine(const String& name, const RegionDefine* rd)
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

/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Colorer Library.
 *
 * The Initial Developer of the Original Code is
 * Cail Lomecb <irusskih at gmail dot com>.
 * Portions created by the Initial Developer are Copyright (C) 1999-2009
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s): see file CONTRIBUTORS
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

