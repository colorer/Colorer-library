#include "ConsoleTools.h"
#include <colorer/common/Encodings.h>
#include <colorer/common/UStr.h>
#include <colorer/cregexp/cregexp.h>
#include <colorer/editor/BaseEditor.h>
#include <colorer/io/FileWriter.h>
#include <colorer/io/InputSource.h>
#include <colorer/viewer/ParsedLineWriter.h>
#include <colorer/viewer/TextConsoleViewer.h>
#include <colorer/xml/XmlParserErrorHandler.h>
#include <ctime>
#include <memory>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

using namespace xercesc;

ConsoleTools::ConsoleTools() = default;

ConsoleTools::~ConsoleTools() = default;

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

void ConsoleTools::setTypeDescription(const UnicodeString& str)
{
  typeDescription = std::make_unique<UnicodeString>(str);
}

void ConsoleTools::setInputFileName(const UnicodeString& str)
{
  inputFileName = std::make_unique<UnicodeString>(str);
}

void ConsoleTools::setOutputFileName(const UnicodeString& str)
{
  outputFileName = std::make_unique<UnicodeString>(str);
}

void ConsoleTools::setCatalogPath(const UnicodeString& str)
{
#if defined _WIN32
  // replace the environment variables to their values
  size_t i = ExpandEnvironmentStrings(UStr::to_stdstr(&str).c_str(), nullptr, 0);
  char* temp = new char[i];
  ExpandEnvironmentStrings(UStr::to_stdstr(&str).c_str(), temp, static_cast<DWORD>(i));
  catalogPath = std::make_unique<UnicodeString>(temp);
  delete[] temp;
#else
  catalogPath.reset(new UnicodeString(str));
#endif
}

void ConsoleTools::setHRDName(const UnicodeString& str)
{
  hrdName = std::make_unique<UnicodeString>(str);
}

void ConsoleTools::setLinkSource(const UnicodeString& str)
{
  const XMLCh kTagDoclinks[] = {chLatin_d, chLatin_o, chLatin_k, chLatin_l, chLatin_i, chLatin_n, chLatin_k, chLatin_s, chNull};
  const XMLCh kTagLinks[] = {chLatin_l, chLatin_i, chLatin_n, chLatin_k, chLatin_s, chNull};
  const XMLCh kTagLink[] = {chLatin_l, chLatin_i, chLatin_n, chLatin_k, chNull};
  const XMLCh kLinksAttrUrl[] = {chLatin_u, chLatin_r, chLatin_l, chNull};
  const XMLCh kLinkAttrUrl[] = {chLatin_u, chLatin_r, chLatin_l, chNull};
  const XMLCh kLinksAttrScheme[] = {chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_e, chNull};
  const XMLCh kLinkAttrScheme[] = {chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_e, chNull};
  const XMLCh kLinkAttrToken[] = {chLatin_t, chLatin_o, chLatin_k, chLatin_e, chLatin_n, chNull};

  uXmlInputSource linkSource = XmlInputSource::newInstance(UStr::to_xmlch(&str).get(), static_cast<XMLCh*>(nullptr));
  xercesc::XercesDOMParser xml_parser;
  XmlParserErrorHandler error_handler;
  xml_parser.setErrorHandler(&error_handler);
  xml_parser.setLoadExternalDTD(false);
  xml_parser.setSkipDTDValidation(true);
  xml_parser.parse(*linkSource->getInputSource());
  if (error_handler.getSawErrors()) {
    throw Exception("Error loading HRD file");
  }
  xercesc::DOMDocument* linkSourceTree = xml_parser.getDocument();
  xercesc::DOMElement* elem = linkSourceTree->getDocumentElement();

  if (elem == nullptr || !xercesc::XMLString::equals(elem->getNodeName(), kTagDoclinks)) {
    throw Exception("Error loading HRD file");
  }

  for (xercesc::DOMNode* curel = elem->getFirstChild(); curel; curel = curel->getNextSibling()) {
    if (curel->getNodeType() == xercesc::DOMNode::ELEMENT_NODE && xercesc::XMLString::equals(curel->getNodeName(), kTagLinks)) {
      auto* subelem = dynamic_cast<xercesc::DOMElement*>(curel);
      if (subelem) {
        const XMLCh* url = subelem->getAttribute(kLinksAttrUrl);
        const XMLCh* scheme = subelem->getAttribute(kLinksAttrScheme);

        for (xercesc::DOMNode* eachLink = curel->getFirstChild(); eachLink; eachLink = eachLink->getNextSibling()) {
          if (eachLink->getNodeType() == xercesc::DOMNode::ELEMENT_NODE && xercesc::XMLString::equals(eachLink->getNodeName(), kTagLink)) {
            auto* subelem2 = dynamic_cast<xercesc::DOMElement*>(eachLink);
            if (subelem2) {
              const XMLCh* l_url = subelem2->getAttribute(kLinkAttrUrl);
              const XMLCh* l_scheme = subelem2->getAttribute(kLinkAttrScheme);
              const XMLCh* token = subelem2->getAttribute(kLinkAttrToken);
              UnicodeString fullURL;
              if (*url != '\0') {
                fullURL.append(UnicodeString(url));
              }
              if (*l_url != '\0') {
                fullURL.append(UnicodeString(l_url));
              }
              if (*l_scheme == '\0') {
                l_scheme = scheme;
              }
              if (*token == '\0') {
                continue;
              }
              auto* tok = new UnicodeString(token);
              UnicodeString hkey(*tok);
              if (*l_scheme != '\0') {
                hkey.append("--").append(UnicodeString(l_scheme));
              }
              std::pair<UnicodeString, UnicodeString*> pair_url(hkey, new UnicodeString(fullURL));
              docLinkHash.emplace(pair_url);
              delete tok;
            }
          }
        }
      }
    }
  }
}

void ConsoleTools::RETest()
{
  SMatches match {};
  CRegExp* re;
  bool res;
  char text[255];

  re = new CRegExp();
  do {
    printf("\nregexp:");
    fgets(text, sizeof(text), stdin);
    strtok(text, "\r\n");
    UnicodeString dtext = UnicodeString(text);
    if (!re->setRE(&dtext)) {
      continue;
    }
    printf("exprn:");
    fgets(text, sizeof(text), stdin);
    strtok(text, "\r\n");
    dtext = UnicodeString(text);
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
    writer = new StreamWriter(stdout, false);
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
        writer->write(*type->getName() + "\n");
      } else {
        if (type->getGroup() != nullptr) {
          writer->write(*type->getGroup() + ": ");
        }
        writer->write(type->getDescription());
        writer->write("\n");
      }

      if (load) {
        hrcParser->loadFileType(type);
      }
    }
    delete writer;
  } catch (Exception& e) {
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
        if (type->getDescription() != nullptr && type->getDescription()->length() >= typeDescription->length() &&
            UnicodeString(*type->getDescription(), 0, typeDescription->length()).caseCompare(*typeDescription, 0)) {
          break;
        }
        if (type->getName()->length() >= typeDescription->length() &&
            UnicodeString(*type->getName(), 0, typeDescription->length()).caseCompare(*typeDescription, 0)) {
          break;
        }
        type = nullptr;
      }
    }
  }
  if (typeDescription == nullptr || type == nullptr) {
    UnicodeString textStart;
    int totalLength = 0;
    for (int i = 0; i < 4; i++) {
      UnicodeString* iLine = lineSource->getLine(i);
      if (iLine == nullptr) {
        break;
      }
      textStart.append(*iLine);
      textStart.append("\n");
      totalLength += iLine->length();
      if (totalLength > 500) {
        break;
      }
    }

    UnicodeString fnpath(*inputFileName);
    auto slash_idx = fnpath.lastIndexOf('\\');

    if (slash_idx == -1) {
      slash_idx = fnpath.lastIndexOf('/');
    }
    std::unique_ptr<UnicodeString> file_name(new UnicodeString(fnpath, slash_idx + 1));

    type = hrcParser->chooseFileType(file_name.get(), &textStart, 0);
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
  textLinesStore.loadFile(inputFileName.get(), true);
  // Base editor to make primary parse
  BaseEditor baseEditor(&pf, &textLinesStore);
  // HRD RegionMapper linking
  UnicodeString dcons = UnicodeString("console");
  baseEditor.setRegionMapper(&dcons, hrdName.get());
  FileType* type = selectType(pf.getHRCParser(), &textLinesStore);
  type->getBaseScheme();
  baseEditor.setFileType(type);

  msecs = clock();
  while (loopCount--) {
    baseEditor.modifyLineEvent(0);
    baseEditor.lineCountEvent((int) textLinesStore.getLineCount());
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
    textLinesStore.loadFile(inputFileName.get(), true);
    // parsers factory
    ParserFactory pf;
    pf.loadCatalog(catalogPath.get());
    // Base editor to make primary parse
    BaseEditor baseEditor(&pf, &textLinesStore);
    // HRD RegionMapper linking
    UnicodeString dcons = UnicodeString("console");
    baseEditor.setRegionMapper(&dcons, hrdName.get());
    FileType* type = selectType(pf.getHRCParser(), &textLinesStore);
    baseEditor.setFileType(type);
    // Initial line count notify
    baseEditor.lineCountEvent((int) textLinesStore.getLineCount());

    unsigned short background;
    const StyledRegion* rd = StyledRegion::cast(baseEditor.rd_def_Text);
    if (rd != nullptr && rd->isForeSet && rd->isBackSet) {
      background = (unsigned short) (rd->fore + (rd->back << 4));
    } else {
      background = 0x1F;
    }
    // File viewing in console window
    TextConsoleViewer viewer(&baseEditor, &textLinesStore, background);
    viewer.view();
  } catch (Exception& e) {
    fprintf(stderr, "%s\n", e.what());
  } catch (...) {
    fprintf(stderr, "unknown exception ...\n");
  }
}

void ConsoleTools::forward()
{
  colorer::InputSource* fis = colorer::InputSource::newInstance(inputFileName.get());
  const byte* stream = fis->openStream();
  auto eStream = Encodings::toUnicodeString((char*) stream, fis->length());

  Writer* outputFile;
  try {
    if (outputFileName != nullptr) {
      outputFile = new FileWriter(outputFileName.get(), bomOutput);
    } else {
      outputFile = new StreamWriter(stdout, bomOutput);
    }
  } catch (Exception& e) {
    fprintf(stderr, "can't open file '%s' for writing:", UStr::to_stdstr(outputFileName.get()).c_str());
    fprintf(stderr, "%s", e.what());
    return;
  }

  outputFile->write(*eStream);

  delete outputFile;
  delete fis;
}

void ConsoleTools::genOutput(bool useTokens)
{
  try {
    // Source file text lines store.
    TextLinesStore textLinesStore;
    textLinesStore.loadFile(inputFileName.get(), true);
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
        UnicodeString drgb = UnicodeString("rgb");
        mapper = pf.createStyledMapper(&drgb, hrdName.get());
      } catch (ParserFactoryException&) {
        useMarkup = true;
        mapper = pf.createTextMapper(hrdName.get());
      }
    }
    // Base editor to make primary parse
    BaseEditor baseEditor(&pf, &textLinesStore);
    // Using compact regions
    baseEditor.setRegionCompact(true);
    baseEditor.setRegionMapper(mapper);
    baseEditor.lineCountEvent((int) textLinesStore.getLineCount());
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
        commonWriter = new FileWriter(outputFileName.get(), bomOutput);
      } else {
        commonWriter = new StreamWriter(stdout, bomOutput);
      }
      if (htmlEscaping) {
        escapedWriter = new HtmlEscapesWriter(commonWriter);
      } else {
        escapedWriter = commonWriter;
      }
    } catch (Exception& e) {
      fprintf(stderr, "can't open file '%s' for writing:\n", UStr::to_stdstr(outputFileName.get()).c_str());
      fprintf(stderr, "%s", e.what());
      return;
    }

    if (htmlWrapping && useTokens) {
      commonWriter->write("<html>\n<head>\n<style></style>\n</head>\n<body><pre>\n");
    } else if (htmlWrapping && rd != nullptr) {
      if (useMarkup) {
        commonWriter->write(*TextRegion::cast(rd)->start_text);
      } else {
        commonWriter->write("<html><body style='");
        ParsedLineWriter::writeStyle(commonWriter, StyledRegion::cast(rd));
        commonWriter->write("'><pre>\n");
      }
    }

    if (copyrightHeader) {
      commonWriter->write("Created with colorer-take5 library. Type '");
      commonWriter->write(type->getName());
      commonWriter->write("'\n\n");
    }

    int lni = 0;
    int lwidth = 1;
    int lncount = (int) textLinesStore.getLineCount();
    for (lni = lncount / 10; lni > 0; lni = lni / 10, lwidth++)
      ;

    for (int i = 0; i < lncount; i++) {
      if (lineNumbers) {
        int iwidth = 1;
        for (lni = i / 10; lni > 0; lni = lni / 10, iwidth++)
          ;
        for (lni = iwidth; lni < lwidth; lni++) {
          commonWriter->write(0x0020);
        }
        commonWriter->write(UStr::to_unistr(i));
        commonWriter->write(": ");
      }
      if (useTokens) {
        ParsedLineWriter::tokenWrite(commonWriter, escapedWriter, &docLinkHash, textLinesStore.getLine(i), baseEditor.getLineRegions(i));
      } else if (useMarkup) {
        ParsedLineWriter::markupWrite(commonWriter, escapedWriter, &docLinkHash, textLinesStore.getLine(i), baseEditor.getLineRegions(i));
      } else {
        ParsedLineWriter::htmlRGBWrite(commonWriter, escapedWriter, &docLinkHash, textLinesStore.getLine(i), baseEditor.getLineRegions(i));
      }
      commonWriter->write("\n");
    }

    if (htmlWrapping && useTokens) {
      commonWriter->write("</pre></body></html>\n");
    } else if (htmlWrapping && rd != nullptr) {
      if (useMarkup) {
        commonWriter->write(*TextRegion::cast(rd)->end_text);
      } else {
        commonWriter->write("</pre></body></html>\n");
      }
    }

    if (htmlEscaping) {
      delete commonWriter;
    }
    delete escapedWriter;

    delete mapper;
  } catch (Exception& e) {
    fprintf(stderr, "%s\n", e.what());
  } catch (...) {
    fprintf(stderr, "unknown exception ...\n");
  }
}

void ConsoleTools::genTokenOutput()
{
  genOutput(true);
}
