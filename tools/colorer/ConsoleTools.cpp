#include<time.h>
#include<colorer/ParserFactory.h>
#include<colorer/editor/BaseEditor.h>
#include<colorer/viewer/TextLinesStore.h>
#include<colorer/viewer/ParsedLineWriter.h>
#include<colorer/viewer/TextConsoleViewer.h>
#include<colorer/handlers/DefaultErrorHandler.h>
#include<colorer/ParserFactoryException.h>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xml/XmlParserErrorHandler.h>
#include <xml/XmlInputSource.h>
#include "ConsoleTools.h"

using namespace xercesc;

ConsoleTools::ConsoleTools(){
  copyrightHeader = true;
  htmlEscaping = true;
  bomOutput = true;
  htmlWrapping = true;
  lineNumbers = false;

  typeDescription = nullptr;
  inputFileName = outputFileName = nullptr;
  inputEncoding = outputEncoding = nullptr;
  inputEncodingIndex = outputEncodingIndex = -1;
  catalogPath = nullptr;
  hrdName = nullptr;
  logFileName = nullptr;

  docLinkHash = new std::unordered_map<SString, String*>;
}

ConsoleTools::~ConsoleTools(){
  delete typeDescription;
  delete catalogPath;
  delete hrdName;
  delete inputEncoding;
  delete outputEncoding;
  delete outputFileName;
  delete inputFileName;
  delete logFileName;

  docLinkHash->clear();
  delete docLinkHash;
}


void ConsoleTools::setCopyrightHeader(bool use) { copyrightHeader = use; }

void ConsoleTools::setHtmlEscaping(bool use) { htmlEscaping = use; }

void ConsoleTools::setBomOutput(bool use) { bomOutput = use; }

void ConsoleTools::setHtmlWrapping(bool use) { htmlWrapping = use; }

void ConsoleTools::addLineNumbers(bool add){ lineNumbers = add; }

colorer::ErrorHandler *ConsoleTools::createErrorHandler() {
  colorer::ErrorHandler *resultHandler = nullptr;
  if (logFileName && logFileName->length()!=0) {
    colorer::InputSource *dfis = colorer::InputSource::newInstance(logFileName);
    try{
      resultHandler = new FileErrorHandler(dfis->getLocation(), Encodings::ENC_UTF8, false);
    }catch(Exception &){
      resultHandler = nullptr;
    }
    delete dfis;
  }
  if (!resultHandler) {
    resultHandler = new DefaultErrorHandler();
  }
  return resultHandler;
}

void ConsoleTools::setTypeDescription(const String &str) {
  delete typeDescription;
  typeDescription = new SString(str);
}

void ConsoleTools::setInputFileName(const String &str) {
  delete inputFileName;
  inputFileName = new SString(str);
}

void ConsoleTools::setOutputFileName(const String &str) {
  delete outputFileName;
  outputFileName = new SString(str);
}

void ConsoleTools::setLogFileName(const String &str) {
  delete logFileName;
  logFileName = new SString(str);
}

void ConsoleTools::setInputEncoding(const String &str) {
  delete inputEncoding;
  inputEncoding = new SString(str);
  inputEncodingIndex = Encodings::getEncodingIndex(inputEncoding->getChars());
  if (inputEncodingIndex == -1) throw Exception(StringBuffer("Unknown input encoding: ")+inputEncoding);
  if (outputEncoding == nullptr) outputEncodingIndex = inputEncodingIndex;
}

void ConsoleTools::setOutputEncoding(const String &str) {
  delete outputEncoding;
  outputEncoding = new SString(str);
  outputEncodingIndex = Encodings::getEncodingIndex(outputEncoding->getChars());
  if (outputEncodingIndex == -1) throw Exception(StringBuffer("Unknown output encoding: ")+outputEncoding);
}

void ConsoleTools::setCatalogPath(const String &str) {
  delete catalogPath;
#if defined _WIN32
   // replace the environment variables to their values
  size_t i=ExpandEnvironmentStrings(str.getWChars(),nullptr,0);
  wchar_t *temp = new wchar_t[i];
  ExpandEnvironmentStrings(str.getWChars(),temp,static_cast<DWORD>(i));
  catalogPath = new SString(temp);
  delete[] temp;
#else
  catalogPath = new SString(str);
#endif
}

void ConsoleTools::setHRDName(const String &str) {
  delete hrdName;
  hrdName = new SString(str);
}

void ConsoleTools::setLinkSource(const String &str){
  const XMLCh kTagDoclinks[] = {chLatin_d, chLatin_o, chLatin_k, chLatin_l, chLatin_i, chLatin_n, chLatin_k, chLatin_s, chNull};
  const XMLCh kTagLinks[] = {chLatin_l, chLatin_i, chLatin_n, chLatin_k, chLatin_s, chNull};
  const XMLCh kTagLink[] = {chLatin_l, chLatin_i, chLatin_n, chLatin_k, chNull};
  const XMLCh kLinksAttrUrl[] = {chLatin_u, chLatin_r, chLatin_l, chNull};
  const XMLCh kLinkAttrUrl[] = {chLatin_u, chLatin_r, chLatin_l, chNull};
  const XMLCh kLinksAttrScheme[] = {chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_e, chNull};
  const XMLCh kLinkAttrScheme[] = {chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_e, chNull};
  const XMLCh kLinkAttrToken[] = {chLatin_t, chLatin_o, chLatin_k, chLatin_e, chLatin_n, chNull};

  XmlInputSource *linkSource = XmlInputSource::newInstance(str.getWChars(), static_cast<XMLCh*>(nullptr));
  xercesc::XercesDOMParser xml_parser;
  XmlParserErrorHandler error_handler;
  xml_parser.setErrorHandler(&error_handler);
  xml_parser.setLoadExternalDTD(false);
  xml_parser.setSkipDTDValidation(true);
  xml_parser.parse(*linkSource->getInputSource());
  if (error_handler.getSawErrors()) {
    throw Exception(DString("Error loading HRD file"));
  }
  xercesc::DOMDocument *linkSourceTree = xml_parser.getDocument();
  xercesc::DOMElement *elem = linkSourceTree->getDocumentElement();

  if (elem == nullptr || !xercesc::XMLString::equals(elem->getNodeName(), kTagDoclinks)) {
    throw Exception(DString("Error loading HRD file"));
  }

  for(xercesc::DOMNode *curel = elem->getFirstChild(); curel; curel = curel->getNextSibling()){
    if (curel->getNodeType() == xercesc::DOMNode::ELEMENT_NODE && xercesc::XMLString::equals(curel->getNodeName(), kTagLinks)){

      xercesc::DOMElement *subelem = static_cast<xercesc::DOMElement*>(curel);
      const XMLCh *url = subelem->getAttribute(kLinksAttrUrl);
      const XMLCh *scheme = subelem->getAttribute(kLinksAttrScheme);

      for(xercesc::DOMNode *eachLink = curel->getFirstChild(); eachLink; eachLink = eachLink->getNextSibling()){
        if (eachLink->getNodeType() == xercesc::DOMNode::ELEMENT_NODE && xercesc::XMLString::equals(eachLink->getNodeName(), kTagLink)){

          xercesc::DOMElement *subelem2 = static_cast<xercesc::DOMElement*>(eachLink);
          const XMLCh *l_url = subelem2->getAttribute(kLinkAttrUrl);
          const XMLCh *l_scheme = subelem2->getAttribute(kLinkAttrScheme);
          const XMLCh *token = subelem2->getAttribute(kLinkAttrToken);
          StringBuffer fullURL;
          if (*url != '\0') fullURL.append(DString(url));
          if (*l_url != '\0') fullURL.append(DString(l_url));
          if (*l_scheme == '\0') l_scheme = scheme;
          if (*token == '\0') continue;
          String *tok = new DString(token);
          StringBuffer hkey(tok);
          if (*l_scheme != '\0'){
            hkey.append(DString("--")).append(DString(l_scheme));
          }
          std::pair<SString, String*> pair_url(&hkey, new SString(&fullURL));
          docLinkHash->emplace(pair_url);
          delete tok;
        }
      }
    }
  }
  delete linkSource;
}


void ConsoleTools::RETest(){
  SMatches match;
  CRegExp *re;
  bool res;
  char text[255];

  re = new CRegExp();
  do{
    printf("\nregexp:");
    fgets(text, sizeof(text), stdin);
    strtok(text, "\r\n");
    DString dtext = DString(text);
    if (!re->setRE(&dtext)) continue;
    printf("exprn:");
    fgets(text, sizeof(text), stdin);
    strtok(text, "\r\n");
    dtext = DString(text);
    res = re->parse(&dtext, &match);
    printf("%s\nmatch:  ",res?"ok":"error");
    for(int i = 0; i < match.cMatch; i++){
      printf("%d:(%d,%d), ",i,match.s[i],match.e[i]);
    }
  }while(text[0]);
  delete re;
}

void ConsoleTools::listTypes(bool load, bool useNames){
  Writer *writer = nullptr;
  colorer::ErrorHandler *err = nullptr;
  try{
    writer = new StreamWriter(stdout, outputEncodingIndex, bomOutput);
    err = createErrorHandler();
    ParserFactory pf(err);
    pf.loadCatalog(catalogPath);
    HRCParser *hrcParser = pf.getHRCParser();
    fprintf(stderr, "\nloading file types...\n");
    for(int idx = 0;; idx++){
      FileType *type = hrcParser->enumerateFileTypes(idx);
      if (type == nullptr) break;
      if (useNames){
        writer->write(StringBuffer(type->getName())+"\n");
      }else{
        if (type->getGroup() != nullptr){
          writer->write(StringBuffer(type->getGroup()) + ": ");
        }
        writer->write(type->getDescription());
        writer->write(DString("\n"));
      }

      if (load) type->getBaseScheme();
    }
    delete writer;
  }catch(Exception &e){
    delete writer;
    fprintf(stderr, "%s\n", e.getMessage()->getChars());
  }
  delete err;
}

FileType *ConsoleTools::selectType(HRCParser *hrcParser, LineSource *lineSource){
  FileType *type = nullptr;
  if (typeDescription != nullptr){
    type = hrcParser->getFileType(typeDescription);
    if (type == nullptr){
      for(int idx = 0;; idx++){
        type = hrcParser->enumerateFileTypes(idx);
        if (type == nullptr) break;
        if (type->getDescription() != nullptr &&
            type->getDescription()->length() >= typeDescription->length() &&
            DString(type->getDescription(), 0, typeDescription->length()).equalsIgnoreCase(typeDescription))
          break;
        if (type->getName()->length() >= typeDescription->length() &&
            DString(type->getName(), 0, typeDescription->length()).equalsIgnoreCase(typeDescription))
          break;
        type = nullptr;
      }
    }
  }
  if (typeDescription == nullptr || type == nullptr){
    StringBuffer textStart;
    int totalLength = 0;
    for(int i = 0; i < 4; i++){
      String *iLine = lineSource->getLine(i);
      if (iLine == nullptr) break;
      textStart.append(iLine);
      textStart.append(DString("\n"));
      totalLength += iLine->length();
      if (totalLength > 500) break;
    }
    type = hrcParser->chooseFileType(inputFileName, &textStart, 0);
  }
  return type;
}

void ConsoleTools::profile(int loopCount){
  clock_t msecs;
  colorer::ErrorHandler *err = createErrorHandler();

  // parsers factory
  ParserFactory pf(err);
  pf.loadCatalog(catalogPath);
  // Source file text lines store.
  TextLinesStore textLinesStore;
  textLinesStore.loadFile(inputFileName, inputEncoding, true);
  // Base editor to make primary parse
  BaseEditor baseEditor(&pf, &textLinesStore);
  // HRD RegionMapper linking
  DString dcons = DString("console");
  baseEditor.setRegionMapper(&dcons, hrdName);
  FileType *type = selectType(pf.getHRCParser(), &textLinesStore);
  type->getBaseScheme();
  baseEditor.setFileType(type);

  msecs = clock();
  while(loopCount--){
    baseEditor.modifyLineEvent(0);
    baseEditor.lineCountEvent(textLinesStore.getLineCount());
    baseEditor.validate(-1, false);
  }
  msecs = clock() - msecs;

  printf("%ld\n", (msecs*1000)/CLOCKS_PER_SEC );
  delete err;
}

void ConsoleTools::viewFile(){
  colorer::ErrorHandler *err = nullptr;
  try{
    // Source file text lines store.
    TextLinesStore textLinesStore;
    textLinesStore.loadFile(inputFileName, inputEncoding, true);
    err = createErrorHandler();
    // parsers factory
    ParserFactory pf(err);
    pf.loadCatalog(catalogPath);
    // Base editor to make primary parse
    BaseEditor baseEditor(&pf, &textLinesStore);
    // HRD RegionMapper linking
    DString dcons = DString("console");
    baseEditor.setRegionMapper(&dcons, hrdName);
    FileType *type = selectType(pf.getHRCParser(), &textLinesStore);
    baseEditor.setFileType(type);
    // Initial line count notify
    baseEditor.lineCountEvent(textLinesStore.getLineCount());

    int background;
    const StyledRegion *rd = StyledRegion::cast(baseEditor.rd_def_Text);
    if (rd != nullptr && rd->bfore && rd->bback) background = rd->fore + (rd->back<<4);
    else background = 0x1F;
    // File viewing in console window
    TextConsoleViewer viewer(&baseEditor, &textLinesStore, background, outputEncodingIndex);
    viewer.view();
  }catch(Exception &e){
    fprintf(stderr, "%s\n", e.getMessage()->getChars());
  }catch(...){
    fprintf(stderr, "unknown exception ...\n");
  }
  delete err;
}

void ConsoleTools::forward(){
  colorer::InputSource *fis = colorer::InputSource::newInstance(inputFileName);
  const byte *stream = fis->openStream();
  DString eStream(stream, fis->length(), inputEncodingIndex);

  Writer *outputFile;
  try{
    if (outputFileName != nullptr) outputFile = new FileWriter(outputFileName, outputEncodingIndex, bomOutput);
    else outputFile = new StreamWriter(stdout, outputEncodingIndex, bomOutput);
  }catch(Exception &e){
    fprintf(stderr, "can't open file '%s' for writing:", outputFileName->getChars());
    fprintf(stderr, e.getMessage()->getChars());
    return;
  }

  outputFile->write(eStream);

  delete outputFile;
  delete fis;
}

void ConsoleTools::genOutput(bool useTokens){
  colorer::ErrorHandler *err = nullptr;
  try{
    // Source file text lines store.
    TextLinesStore textLinesStore;
    textLinesStore.loadFile(inputFileName, inputEncoding, true);
    err = createErrorHandler();
    // parsers factory
    ParserFactory pf(err);
    pf.loadCatalog(catalogPath);
    // HRC loading
    HRCParser *hrcParser = pf.getHRCParser();
    // HRD RegionMapper creation
    bool useMarkup = false;
    RegionMapper *mapper = nullptr;
    if (!useTokens){
      try{
        DString drgb = DString("rgb");
        mapper = pf.createStyledMapper(&drgb, hrdName);
      }catch(ParserFactoryException &){
        useMarkup = true;
        mapper = pf.createTextMapper(hrdName);
      }
    }
    // Base editor to make primary parse
    BaseEditor baseEditor(&pf, &textLinesStore);
    // Using compact regions
    baseEditor.setRegionCompact(true);
    baseEditor.setRegionMapper(mapper);
    baseEditor.lineCountEvent(textLinesStore.getLineCount());
    // Choosing file type
    FileType *type = selectType(hrcParser, &textLinesStore);
    baseEditor.setFileType(type);

    //  writing result into HTML colored stream...
    const RegionDefine *rd = nullptr;
    if (mapper != nullptr) rd = baseEditor.rd_def_Text;

    Writer *escapedWriter;
    Writer *commonWriter;
    try{
      if (outputFileName != nullptr) commonWriter = new FileWriter(outputFileName, outputEncodingIndex, bomOutput);
      else commonWriter = new StreamWriter(stdout, outputEncodingIndex, bomOutput);
      if (htmlEscaping) escapedWriter = new HtmlEscapesWriter(commonWriter);
      else escapedWriter = commonWriter;
    }catch(Exception &e){
      fprintf(stderr, "can't open file '%s' for writing:\n", outputFileName->getChars());
      fprintf(stderr, e.getMessage()->getChars());
      delete err;
      return;
    }

    if (htmlWrapping && useTokens){
      commonWriter->write(DString("<html>\n<head>\n<style></style>\n</head>\n<body><pre>\n"));
    }else if (htmlWrapping && rd != nullptr){
      if (useMarkup){
        commonWriter->write(TextRegion::cast(rd)->stext);
      }else{
        commonWriter->write(DString("<html><body style='"));
        ParsedLineWriter::writeStyle(commonWriter, StyledRegion::cast(rd));
        commonWriter->write(DString("'><pre>\n"));
      }
    }

    if (copyrightHeader){
      commonWriter->write(DString("Created with colorer-take5 library. Type '"));
      commonWriter->write(type->getName());
      commonWriter->write(DString("'\n\n"));
    }

    int lni = 0;
    int lwidth = 1;
    int lncount = textLinesStore.getLineCount();
    for(lni = lncount/10; lni > 0; lni = lni/10, lwidth++);

    for(int i = 0; i < lncount; i++){
      if (lineNumbers){
        int iwidth = 1;
        for(lni = i/10; lni > 0; lni = lni/10, iwidth++);
        for(lni = iwidth; lni < lwidth; lni++) commonWriter->write(0x0020);
        commonWriter->write(SString(i));
        commonWriter->write(DString(": "));
      }
      if (useTokens){
        ParsedLineWriter::tokenWrite(commonWriter, escapedWriter, docLinkHash, textLinesStore.getLine(i), baseEditor.getLineRegions(i));
      }else if (useMarkup){
        ParsedLineWriter::markupWrite(commonWriter, escapedWriter, docLinkHash, textLinesStore.getLine(i), baseEditor.getLineRegions(i));
      }else{
        ParsedLineWriter::htmlRGBWrite(commonWriter, escapedWriter, docLinkHash, textLinesStore.getLine(i), baseEditor.getLineRegions(i));
      }
      commonWriter->write(DString("\n"));
    }

    if (htmlWrapping && useTokens){
      commonWriter->write(DString("</pre></body></html>\n"));
    }else if (htmlWrapping && rd != nullptr){
      if (useMarkup){
        commonWriter->write(TextRegion::cast(rd)->etext);
      }else{
        commonWriter->write(DString("</pre></body></html>\n"));
      }
    }

    if (htmlEscaping) delete commonWriter;
    delete escapedWriter;

    delete mapper;
  }catch(Exception &e){
    fprintf(stderr, "%s\n", e.getMessage()->getChars());
  }catch(...){
    fprintf(stderr, "unknown exception ...\n");
  }
  delete err;
}

void ConsoleTools::genTokenOutput(){
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
 * The Original Code is the Colorer Library
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