#include <stdio.h>
#include <colorer/handlers/TextHRDMapper.h>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xml/XmlParserErrorHandler.h>
#include <xml/XmlTagDefs.h>

TextHRDMapper::TextHRDMapper() {}
TextHRDMapper::~TextHRDMapper()
{
  for (auto rdef = regionDefines.begin(); rdef != regionDefines.end(); ++rdef) {
    const TextRegion* rd = TextRegion::cast(rdef->second);
    delete rd->stext;
    delete rd->etext;
    delete rd->sback;
    delete rd->eback;
    delete rd;
  }
}

/** Loads region definitions from HRD file.
    Multiple files could be loaded.
*/
void TextHRDMapper::loadRegionMappings(XmlInputSource* is, colorer::ErrorHandler* eh)
{
  xercesc::XercesDOMParser xml_parser;
  XmlParserErrorHandler error_handler(eh);
  xml_parser.setErrorHandler(&error_handler);
  xml_parser.setLoadExternalDTD(false);
  xml_parser.setSkipDTDValidation(true);
  xml_parser.parse(*is->getInputSource());
  if (error_handler.getSawErrors()) {
    throw Exception(DString("Error loading HRD file"));
  }
  xercesc::DOMDocument* hrdbase = xml_parser.getDocument();
  xercesc::DOMElement* hbase = hrdbase->getDocumentElement();

  if (hbase == null || !xercesc::XMLString::equals(hbase->getNodeName(), hrdTagHrd)) {
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
      auto tp = regionDefines.find(name);
      if (tp != regionDefines.end()) {
        const TextRegion* rd = TextRegion::cast(tp->second);
        delete rd->stext;
        delete rd->etext;
        delete rd->sback;
        delete rd->eback;
        delete rd;
      }
      const String* stext = null;
      const String* etext = null;
      const String* sback = null;
      const String* eback = null;
      const XMLCh* sval;
      sval = subelem->getAttribute(hrdAssignAttrSText);
      if (*sval != '\0') {
        stext = new SString(DString(sval));
      }
      sval = subelem->getAttribute(hrdAssignAttrEText);
      if (*sval != '\0') {
        etext = new SString(DString(sval));
      }
      sval = subelem->getAttribute(hrdAssignAttrSBack);
      if (*sval != '\0') {
        sback = new SString(DString(sval));
      }
      sval = subelem->getAttribute(hrdAssignAttrEBack);
      if (*sval != '\0') {
        eback = new SString(DString(sval));
      }

      RegionDefine* rdef = new TextRegion(stext, etext, sback, eback);
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
void TextHRDMapper::saveRegionMappings(Writer* writer) const
{
  writer->write(DString("<?xml version=\"1.0\"?>\n\
<!DOCTYPE hrd SYSTEM \"../hrd.dtd\">\n\n\
<hrd>\n"));
  for (auto it = regionDefines.begin(); it != regionDefines.end(); ++it) {
    const TextRegion* rdef = TextRegion::cast(it->second);
    writer->write(StringBuffer("  <define name='") + it->first + "'");
    if (rdef->stext != null) {
      writer->write(StringBuffer(" stext='") + rdef->stext + "'");
    }
    if (rdef->etext != null) {
      writer->write(StringBuffer(" etext='") + rdef->etext + "'");
    }
    if (rdef->sback != null) {
      writer->write(StringBuffer(" sback='") + rdef->sback + "'");
    }
    if (rdef->eback != null) {
      writer->write(StringBuffer(" eback='") + rdef->eback + "'");
    }
    writer->write(DString("/>\n"));
  }
  writer->write(DString("\n</hrd>\n"));
}

/** Adds or replaces region definition */
void TextHRDMapper::setRegionDefine(const String& name, const RegionDefine* rd)
{
  const TextRegion* rd_new = TextRegion::cast(rd);
  const String* stext = null;
  const String* etext = null;
  const String* sback = null;
  const String* eback = null;
  if (rd_new->stext != null) {
    stext = new SString(rd_new->stext);
  }
  if (rd_new->etext != null) {
    etext = new SString(rd_new->etext);
  }
  if (rd_new->sback != null) {
    sback = new SString(rd_new->sback);
  }
  if (rd_new->eback != null) {
    eback = new SString(rd_new->eback);
  }

  auto rd_old = regionDefines.find(name);
  if (rd_old != regionDefines.end()) {
    const TextRegion* rdef = TextRegion::cast(rd_old->second);
    delete rdef->stext;
    delete rdef->etext;
    delete rdef->sback;
    delete rdef->eback;
    delete rdef;
  }

  RegionDefine* new_region = new TextRegion(stext, etext, sback, eback);
  std::pair<SString, RegionDefine*> p(name, new_region);
  regionDefines.emplace(p);

  // Searches and replaces old region references
  for (size_t idx = 0; idx < regionDefinesVector.size(); idx++)
    if (regionDefinesVector.at(idx) == rd_old->second) {
      regionDefinesVector.at(idx) = new_region;
      break;
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
 * Cail Lomecb <cail@nm.ru>.
 * Portions created by the Initial Developer are Copyright (C) 1999-2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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
