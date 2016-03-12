#include <time.h>
#include <colorer/parsers/ParserFactory.h>
#include <colorer/editor/BaseEditor.h>
#include <colorer/viewer/TextLinesStore.h>
#include <colorer/viewer/ParsedLineWriter.h>
#include <colorer/viewer/TextConsoleViewer.h>
#include <colorer/parsers/ParserFactoryException.h>
#include <colorer/io/FileWriter.h>
#include <colorer/io/InputSource.h>
#include <colorer/cregexp/cregexp.h>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <colorer/xml/XmlParserErrorHandler.h>
#include <colorer/xml/XmlInputSource.h>
#include <colorer/unicode/Encodings.h>
#include "ConsoleTools.h"

using namespace xercesc;

ConsoleTools::ConsoleTools(): copyrightHeader(true), htmlEscaping(true), bomOutput(true), htmlWrapping(true), lineNumbers(false),
  inputEncodingIndex(-1), outputEncodingIndex(-1), inputEncoding(nullptr), outputEncoding(nullptr), typeDescription(nullptr), catalogPath(nullptr), hrdName(nullptr),
  outputFileName(nullptr), inputFileName(nullptr)
{
}

ConsoleTools::~ConsoleTools()
{
}

void ConsoleTools::setCopyrightHeader(bool use)
{
  copyrightHeader = use;
}

void ConsoleTools::setHtmlEscaping(bool use)
{
  htmlEscaping = use;
}

void ConsoleTools::setBomOutput(bool use)
{
  bomOutput = use;
}

void ConsoleTools::setHtmlWrapping(bool use)
{
  htmlWrapping = use;
}

void ConsoleTools::addLineNumbers(bool add)
{
  lineNumbers = add;
}

void ConsoleTools::setTypeDescription(const String &str)
{
  typeDescription.reset(new SString(str));
}

void ConsoleTools::setInputFileName(const String &str)
{
  inputFileName.reset(new SString(str));
}

void ConsoleTools::setOutputFileName(const String &str)
{
  outputFileName.reset(new SString(str));
}

void ConsoleTools::setInputEncoding(const String &str)
{
  inputEncoding.reset(new SString(str));
  inputEncodingIndex = Encodings::getEncodingIndex(inputEncoding->getChars());
  if (inputEncodingIndex == -1) {
    throw Exception(SString("Unknown input encoding: ") + inputEncoding.get());
  }
  if (outputEncoding == nullptr) {
    outputEncodingIndex = inputEncodingIndex;
  }
}

void ConsoleTools::setOutputEncoding(const String &str)
{
  outputEncoding.reset(new SString(str));
  outputEncodingIndex = Encodings::getEncodingIndex(outputEncoding->getChars());
  if (outputEncodingIndex == -1) {
    throw Exception(SString("Unknown output encoding: ") + outputEncoding.get());
  }
}

void ConsoleTools::setCatalogPath(const String &str)
{
#if defined _WIN32
  // replace the environment variables to their values
  size_t i = ExpandEnvironmentStrings(str.getWChars(), nullptr, 0);
  wchar_t* temp = new wchar_t[i];
  ExpandEnvironmentStrings(str.getWChars(), temp, static_cast<DWORD>(i));
  catalogPath.reset(new SString(temp));
  delete[] temp;
#else
  catalogPath.reset(new SString(str));
#endif
}

void ConsoleTools::setHRDName(const String &str)
{
  hrdName.reset(new SString(str));
}

void ConsoleTools::setLinkSource(const String &str)
{
  const XMLCh kTagDoclinks[] = {chLatin_d, chLatin_o, chLatin_k, chLatin_l, chLatin_i, chLatin_n, chLatin_k, chLatin_s, chNull};
  const XMLCh kTagLinks[] = {chLatin_l, chLatin_i, chLatin_n, chLatin_k, chLatin_s, chNull};
  const XMLCh kTagLink[] = {chLatin_l, chLatin_i, chLatin_n, chLatin_k, chNull};
  const XMLCh kLinksAttrUrl[] = {chLatin_u, chLatin_r, chLatin_l, chNull};
  const XMLCh kLinkAttrUrl[] = {chLatin_u, chLatin_r, chLatin_l, chNull};
  const XMLCh kLinksAttrScheme[] = {chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_e, chNull};
  const XMLCh kLinkAttrScheme[] = {chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_e, chNull};
  const XMLCh kLinkAttrToken[] = {chLatin_t, chLatin_o, chLatin_k, chLatin_e, chLatin_n, chNull};

  uXmlInputSource linkSource = XmlInputSource::newInstance(str.getWChars(), static_cast<XMLCh*>(nullptr));
  xercesc::XercesDOMParser xml_parser;
  XmlParserErrorHandler error_handler;
  xml_parser.setErrorHandler(&error_handler);
  xml_parser.setLoadExternalDTD(false);
  xml_parser.setSkipDTDValidation(true);
  xml_parser.parse(*linkSource->getInputSource());
  if (error_handler.getSawErrors()) {
    throw Exception(DString("Error loading HRD file"));
  }
  xercesc::DOMDocument* linkSourceTree = xml_parser.getDocument();
  xercesc::DOMElement* elem = linkSourceTree->getDocumentElement();

  if (elem == nullptr || !xercesc::XMLString::equals(elem->getNodeName(), kTagDoclinks)) {
    throw Exception(DString("Error loading HRD file"));
  }

  for (xercesc::DOMNode* curel = elem->getFirstChild(); curel; curel = curel->getNextSibling()) {
    if (curel->getNodeType() == xercesc::DOMNode::ELEMENT_NODE && xercesc::XMLString::equals(curel->getNodeName(), kTagLinks)) {

      xercesc::DOMElement* subelem = static_cast<xercesc::DOMElement*>(curel);
      const XMLCh* url = subelem->getAttribute(kLinksAttrUrl);
      const XMLCh* scheme = subelem->getAttribute(kLinksAttrScheme);

      for (xercesc::DOMNode* eachLink = curel->getFirstChild(); eachLink; eachLink = eachLink->getNextSibling()) {
        if (eachLink->getNodeType() == xercesc::DOMNode::ELEMENT_NODE && xercesc::XMLString::equals(eachLink->getNodeName(), kTagLink)) {

          xercesc::DOMElement* subelem2 = static_cast<xercesc::DOMElement*>(eachLink);
          const XMLCh* l_url = subelem2->getAttribute(kLinkAttrUrl);
          const XMLCh* l_scheme = subelem2->getAttribute(kLinkAttrScheme);
          const XMLCh* token = subelem2->getAttribute(kLinkAttrToken);
          SString fullURL;
          if (*url != '\0') {
            fullURL.append(DString(url));
          }
          if (*l_url != '\0') {
            fullURL.append(DString(l_url));
          }
          if (*l_scheme == '\0') {
            l_scheme = scheme;
          }
          if (*token == '\0') {
            continue;
          }
          String* tok = new DString(token);
          SString hkey(tok);
          if (*l_scheme != '\0') {
            hkey.append(DString("--")).append(DString(l_scheme));
          }
          std::pair<SString, String*> pair_url(&hkey, new SString(&fullURL));
          docLinkHash.emplace(pair_url);
          delete tok;
        }
      }
    }
  }
}


void ConsoleTools::RETest()
{
  SMatches match;
  CRegExp* re;
  bool res;
  char text[255];

  re = new CRegExp();
  do {
    printf("\nregexp:");
    fgets(text, sizeof(text), stdin);
    strtok(text, "\r\n");
    DString dtext = DString(text);
    if (!re->setRE(&dtext)) {
      continue;
    }
    printf("exprn:");
    fgets(text, sizeof(text), stdin);
    strtok(text, "\r\n");
    dtext = DString(text);
    res = re->parse(&dtext, &match);
    printf("%s\nmatch:  ", res ? "ok" : "error");
    for (int i = 0; i < match.cMatch; i++) {
      printf("%d:(%d,%d), ", i, match.s[i], match.e[i]);
    }
  } while (text[0]);
  delete re;
}

void ConsoleTools::listTypes(bool load, bool useNames)
{
  Writer* writer = nullptr;
  try {
    writer = new StreamWriter(stdout, outputEncodingIndex, bomOutput);
    ParserFactory pf;
    pf.loadCatalog(catalogPath.get());
    HRCParser* hrcParser = pf.getHRCParser();
    fprintf(stderr, "\nloading file types...\n");
    for (int idx = 0;; idx++) {
      FileType* type = hrcParser->enumerateFileTypes(idx);
      if (type == nullptr) {
        break;
      }
      if (useNames) {
        writer->write(SString(type->getName()) + "\n");
      } else {
        if (type->getGroup() != nullptr) {
          writer->write(SString(type->getGroup()) + ": ");
        }
        writer->write(type->getDescription());
        writer->write(DString("\n"));
      }

      if (load) {
        type->getBaseScheme();
      }
    }
    delete writer;
  } catch (Exception &e) {
    delete writer;
    fprintf(stderr, "%s\n", e.what());
  }
}

FileType* ConsoleTools::selectType(HRCParser* hrcParser, LineSource* lineSource)
{
  FileType* type = nullptr;
  if (typeDescription != nullptr) {
    type = hrcParser->getFileType(typeDescription.get());
    if (type == nullptr) {
      for (int idx = 0;; idx++) {
        type = hrcParser->enumerateFileTypes(idx);
        if (type == nullptr) {
          break;
        }
        if (type->getDescription() != nullptr &&
            type->getDescription()->length() >= typeDescription->length() &&
            DString(type->getDescription(), 0, typeDescription->length()).equalsIgnoreCase(typeDescription.get())) {
          break;
        }
        if (type->getName()->length() >= typeDescription->length() &&
            DString(type->getName(), 0, typeDescription->length()).equalsIgnoreCase(typeDescription.get())) {
          break;
        }
        type = nullptr;
      }
    }
  }
  if (typeDescription == nullptr || type == nullptr) {
    SString textStart;
    int totalLength = 0;
    for (int i = 0; i < 4; i++) {
      String* iLine = lineSource->getLine(i);
      if (iLine == nullptr) {
        break;
      }
      textStart.append(iLine);
      textStart.append(DString("\n"));
      totalLength += iLine->length();
      if (totalLength > 500) {
        break;
      }
    }
    type = hrcParser->chooseFileType(inputFileName.get(), &textStart, 0);
  }
  return type;
}

void ConsoleTools::profile(int loopCount)
{
  clock_t msecs;

  // parsers factory
  ParserFactory pf;
  pf.loadCatalog(catalogPath.get());
  // Source file text lines store.
  TextLinesStore textLinesStore;
  textLinesStore.loadFile(inputFileName.get(), inputEncoding.get(), true);
  // Base editor to make primary parse
  BaseEditor baseEditor(&pf, &textLinesStore);
  // HRD RegionMapper linking
  DString dcons = DString("console");
  baseEditor.setRegionMapper(&dcons, hrdName.get());
  FileType* type = selectType(pf.getHRCParser(), &textLinesStore);
  type->getBaseScheme();
  baseEditor.setFileType(type);

  msecs = clock();
  while (loopCount--) {
    baseEditor.modifyLineEvent(0);
    baseEditor.lineCountEvent(textLinesStore.getLineCount());
    baseEditor.validate(-1, false);
  }
  msecs = clock() - msecs;

  printf("%ld\n", (msecs * 1000) / CLOCKS_PER_SEC);
}

void ConsoleTools::viewFile()
{
  try {
    // Source file text lines store.
    TextLinesStore textLinesStore;
    textLinesStore.loadFile(inputFileName.get(), inputEncoding.get(), true);
    // parsers factory
    ParserFactory pf;
    pf.loadCatalog(catalogPath.get());
    // Base editor to make primary parse
    BaseEditor baseEditor(&pf, &textLinesStore);
    // HRD RegionMapper linking
    DString dcons = DString("console");
    baseEditor.setRegionMapper(&dcons, hrdName.get());
    FileType* type = selectType(pf.getHRCParser(), &textLinesStore);
    baseEditor.setFileType(type);
    // Initial line count notify
    baseEditor.lineCountEvent(textLinesStore.getLineCount());

    int background;
    const StyledRegion* rd = StyledRegion::cast(baseEditor.rd_def_Text);
    if (rd != nullptr && rd->bfore && rd->bback) {
      background = rd->fore + (rd->back << 4);
    } else {
      background = 0x1F;
    }
    // File viewing in console window
    TextConsoleViewer viewer(&baseEditor, &textLinesStore, background, outputEncodingIndex);
    viewer.view();
  } catch (Exception &e) {
    fprintf(stderr, "%s\n", e.what());
  } catch (...) {
    fprintf(stderr, "unknown exception ...\n");
  }
}

void ConsoleTools::forward()
{
  colorer::InputSource* fis = colorer::InputSource::newInstance(inputFileName.get());
  const byte* stream = fis->openStream();
  DString eStream(stream, fis->length(), inputEncodingIndex);

  Writer* outputFile;
  try {
    if (outputFileName != nullptr) {
      outputFile = new FileWriter(outputFileName.get(), outputEncodingIndex, bomOutput);
    } else {
      outputFile = new StreamWriter(stdout, outputEncodingIndex, bomOutput);
    }
  } catch (Exception &e) {
    fprintf(stderr, "can't open file '%s' for writing:", outputFileName->getChars());
    fprintf(stderr, "%s", e.what());
    return;
  }

  outputFile->write(eStream);

  delete outputFile;
  delete fis;
}

void ConsoleTools::genOutput(bool useTokens)
{
  try {
    // Source file text lines store.
    TextLinesStore textLinesStore;
    textLinesStore.loadFile(inputFileName.get(), inputEncoding.get(), true);
    // parsers factory
    ParserFactory pf;
    pf.loadCatalog(catalogPath.get());
    // HRC loading
    HRCParser* hrcParser = pf.getHRCParser();
    // HRD RegionMapper creation
    bool useMarkup = false;
    RegionMapper* mapper = nullptr;
    if (!useTokens) {
      try {
        DString drgb = DString("rgb");
        mapper = pf.createStyledMapper(&drgb, hrdName.get());
      } catch (ParserFactoryException &) {
        useMarkup = true;
        mapper = pf.createTextMapper(hrdName.get());
      }
    }
    // Base editor to make primary parse
    BaseEditor baseEditor(&pf, &textLinesStore);
    // Using compact regions
    baseEditor.setRegionCompact(true);
    baseEditor.setRegionMapper(mapper);
    baseEditor.lineCountEvent(textLinesStore.getLineCount());
    // Choosing file type
    FileType* type = selectType(hrcParser, &textLinesStore);
    baseEditor.setFileType(type);

    //  writing result into HTML colored stream...
    const RegionDefine* rd = nullptr;
    if (mapper != nullptr) {
      rd = baseEditor.rd_def_Text;
    }

    Writer* escapedWriter;
    Writer* commonWriter;
    try {
      if (outputFileName != nullptr) {
        commonWriter = new FileWriter(outputFileName.get(), outputEncodingIndex, bomOutput);
      } else {
        commonWriter = new StreamWriter(stdout, outputEncodingIndex, bomOutput);
      }
      if (htmlEscaping) {
        escapedWriter = new HtmlEscapesWriter(commonWriter);
      } else {
        escapedWriter = commonWriter;
      }
    } catch (Exception &e) {
      fprintf(stderr, "can't open file '%s' for writing:\n", outputFileName->getChars());
      fprintf(stderr, "%s", e.what());
      return;
    }

    if (htmlWrapping && useTokens) {
      commonWriter->write(DString("<html>\n<head>\n<style></style>\n</head>\n<body><pre>\n"));
    } else if (htmlWrapping && rd != nullptr) {
      if (useMarkup) {
        commonWriter->write(TextRegion::cast(rd)->start_text);
      } else {
        commonWriter->write(DString("<html><body style='"));
        ParsedLineWriter::writeStyle(commonWriter, StyledRegion::cast(rd));
        commonWriter->write(DString("'><pre>\n"));
      }
    }

    if (copyrightHeader) {
      commonWriter->write(DString("Created with colorer-take5 library. Type '"));
      commonWriter->write(type->getName());
      commonWriter->write(DString("'\n\n"));
    }

    int lni = 0;
    int lwidth = 1;
    int lncount = textLinesStore.getLineCount();
    for (lni = lncount / 10; lni > 0; lni = lni / 10, lwidth++);

    for (int i = 0; i < lncount; i++) {
      if (lineNumbers) {
        int iwidth = 1;
        for (lni = i / 10; lni > 0; lni = lni / 10, iwidth++);
        for (lni = iwidth; lni < lwidth; lni++) {
          commonWriter->write(0x0020);
        }
        commonWriter->write(SString(i));
        commonWriter->write(DString(": "));
      }
      if (useTokens) {
        ParsedLineWriter::tokenWrite(commonWriter, escapedWriter, &docLinkHash, textLinesStore.getLine(i), baseEditor.getLineRegions(i));
      } else if (useMarkup) {
        ParsedLineWriter::markupWrite(commonWriter, escapedWriter, &docLinkHash, textLinesStore.getLine(i), baseEditor.getLineRegions(i));
      } else {
        ParsedLineWriter::htmlRGBWrite(commonWriter, escapedWriter, &docLinkHash, textLinesStore.getLine(i), baseEditor.getLineRegions(i));
      }
      commonWriter->write(DString("\n"));
    }

    if (htmlWrapping && useTokens) {
      commonWriter->write(DString("</pre></body></html>\n"));
    } else if (htmlWrapping && rd != nullptr) {
      if (useMarkup) {
        commonWriter->write(TextRegion::cast(rd)->end_text);
      } else {
        commonWriter->write(DString("</pre></body></html>\n"));
      }
    }

    if (htmlEscaping) {
      delete commonWriter;
    }
    delete escapedWriter;

    delete mapper;
  } catch (Exception &e) {
    fprintf(stderr, "%s\n", e.what());
  } catch (...) {
    fprintf(stderr, "unknown exception ...\n");
  }
}

void ConsoleTools::genTokenOutput()
{
  genOutput(true);
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