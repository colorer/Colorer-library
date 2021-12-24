#include "ConsoleTools.h"
#include <colorer/base/BaseNames.h>
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
  catalogPath = std::make_unique<UnicodeString>(str);
}

void ConsoleTools::setHRDName(const UnicodeString& str)
{
  hrdName = std::make_unique<UnicodeString>(str);
}

void ConsoleTools::setLinkSource(const UnicodeString& str)
{
  const char16_t kTagDoclinks[] = u"doclinks\0";
  const char16_t kTagLinks[] = u"links\0";
  const char16_t kTagLink[] = u"link\0";
  const char16_t kLinksAttrUrl[] = u"url\0";
  const char16_t kLinkAttrUrl[] = u"url\0";
  const char16_t kLinksAttrScheme[] = u"scheme\0";
  const char16_t kLinkAttrScheme[] = u"scheme\0";
  const char16_t kLinkAttrToken[] = u"tokrn\0";

  uXmlInputSource linkSource = XmlInputSource::newInstance(&str);
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
    if (curel->getNodeType() == xercesc::DOMNode::ELEMENT_NODE &&
        xercesc::XMLString::equals(curel->getNodeName(), kTagLinks))
    {
      auto* subelem = dynamic_cast<xercesc::DOMElement*>(curel);
      if (subelem) {
        const XMLCh* url = subelem->getAttribute(kLinksAttrUrl);
        const XMLCh* scheme = subelem->getAttribute(kLinksAttrScheme);

        for (xercesc::DOMNode* eachLink = curel->getFirstChild(); eachLink;
             eachLink = eachLink->getNextSibling())
        {
          if (eachLink->getNodeType() == xercesc::DOMNode::ELEMENT_NODE &&
              xercesc::XMLString::equals(eachLink->getNodeName(), kTagLink))
          {
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
  CRegExp re;
  bool res;
  char text[255];

  do {
    printf("\nregexp:");
    if (fgets(text, sizeof(text), stdin) == nullptr) {
      continue;
    }
    strtok(text, "\r\n");
    UnicodeString dtext = UnicodeString(text);
    if (!re.setRE(&dtext)) {
      continue;
    }
    printf("exprn:");
    if (fgets(text, sizeof(text), stdin) == nullptr) {
      continue;
    }
    strtok(text, "\r\n");
    dtext = UnicodeString(text);
    res = re.parse(&dtext, &match);
    printf("%s\nmatch:  ", res ? "ok" : "error");
    for (int i = 0; i < match.cMatch; i++) {
      printf("%d:(%d,%d), ", i, match.s[i], match.e[i]);
    }
  } while (text[0]);
}

void ConsoleTools::listTypes(bool load, bool useNames)
{
  Writer* writer = nullptr;
  try {
    writer = new StreamWriter(stdout, false);
    ParserFactory pf;
    pf.loadCatalog(catalogPath.get());
    auto& hrcLibrary = pf.getHrcLibrary();
    fprintf(stdout, "loading file types...\n");
    for (int idx = 0;; idx++) {
      FileType* type = hrcLibrary.enumerateFileTypes(idx);
      if (type == nullptr) {
        break;
      }
      if (useNames) {
        writer->write(type->getName() + "\n");
      }
      else {
        if (type->getGroup() != nullptr) {
          writer->write(type->getGroup() + ": ");
        }
        writer->write(type->getDescription());
        writer->write("\n");
      }

      if (load) {
        hrcLibrary.loadFileType(type);
      }
    }
    delete writer;
  } catch (Exception& e) {
    delete writer;
    fprintf(stderr, "%s\n", e.what());
  }
}

FileType* ConsoleTools::selectType(HrcLibrary* hrcLibrary, LineSource* lineSource)
{
  FileType* type = nullptr;
  if (typeDescription) {
    type = hrcLibrary->getFileType(typeDescription.get());
    if (type == nullptr) {
      // don`t found type by name, check by description or part of description
      for (int idx = 0;; idx++) {
        type = hrcLibrary->enumerateFileTypes(idx);
        if (type == nullptr) {
          break;
        }
        if (type->getDescription().length() >= typeDescription->length() &&
            UnicodeString(type->getDescription(), 0, typeDescription->length())
                .caseCompare(*typeDescription, U_FOLD_CASE_DEFAULT))
        {
          break;
        }
        if (type->getName().length() >= typeDescription->length() &&
            UnicodeString(type->getName(), 0, typeDescription->length())
                .caseCompare(*typeDescription, U_FOLD_CASE_DEFAULT))
        {
          break;
        }
      }
    }
    if (type == nullptr) {
      spdlog::warn("Don`t found type by name '{0}'", *typeDescription);
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

    std::unique_ptr<UnicodeString> file_name;
    if (inputFileName) {
      UnicodeString fnpath(*inputFileName);
      auto slash_idx = fnpath.lastIndexOf('\\');

      if (slash_idx == -1) {
        slash_idx = fnpath.lastIndexOf('/');
      }
      file_name = std::make_unique<UnicodeString>(fnpath, slash_idx + 1);
    }
    type = hrcLibrary->chooseFileType(file_name.get(), &textStart, 0);
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
  FileType* type = selectType(&pf.getHrcLibrary(), &textLinesStore);
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
    FileType* type = selectType(&pf.getHrcLibrary(), &textLinesStore);
    baseEditor.setFileType(type);
    // Initial line count notify
    baseEditor.lineCountEvent((int) textLinesStore.getLineCount());

    unsigned short background;
    const StyledRegion* rd = StyledRegion::cast(baseEditor.rd_def_Text);
    if (rd != nullptr && rd->isForeSet && rd->isBackSet) {
      background = (unsigned short) (rd->fore + (rd->back << 4));
    }
    else {
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
    }
    else {
      outputFile = new StreamWriter(stdout, bomOutput);
    }
  } catch (Exception& e) {
    fprintf(stderr,
            "can't open file '%s' for writing:", UStr::to_stdstr(outputFileName.get()).c_str());
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
    auto& hrcLibrary = pf.getHrcLibrary();
    // HRD RegionMapper creation
    bool useMarkup = false;
    std::unique_ptr<RegionMapper> mapper;
    if (!useTokens) {
      try {
        UnicodeString drgb = UnicodeString(HrdClassRgb);
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
    baseEditor.setRegionMapper(mapper.get());
    baseEditor.lineCountEvent((int) textLinesStore.getLineCount());
    // Choosing file type
    FileType* type = selectType(&hrcLibrary, &textLinesStore);
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
      }
      else {
        commonWriter = new StreamWriter(stdout, bomOutput);
      }
      if (htmlEscaping) {
        escapedWriter = new HtmlEscapesWriter(commonWriter);
      }
      else {
        escapedWriter = commonWriter;
      }
    } catch (Exception& e) {
      fprintf(stderr, "can't open file '%s' for writing:\n",
              UStr::to_stdstr(outputFileName.get()).c_str());
      fprintf(stderr, "%s", e.what());
      return;
    }

    if (htmlWrapping && useTokens) {
      commonWriter->write("<html>\n<head>\n<style></style>\n</head>\n<body><pre>\n");
    }
    else if (htmlWrapping && rd != nullptr) {
      if (useMarkup) {
        commonWriter->write(*TextRegion::cast(rd)->start_text);
      }
      else {
        commonWriter->write("<html><body style='");
        ParsedLineWriter::writeStyle(commonWriter, StyledRegion::cast(rd));
        commonWriter->write("'><pre>\n");
      }
    }

    if (copyrightHeader) {
      commonWriter->write("Created with colorer library. Type '");
      commonWriter->write(type->getName());
      commonWriter->write("'\n\n");
    }

    int lni = 0;
    int lwidth = 1;
    int lncount = (int) textLinesStore.getLineCount();
    for (lni = lncount / 10; lni > 0; lni = lni / 10) {
      lwidth++;
    }
    for (int i = 0; i < lncount; i++) {
      if (lineNumbers) {
        int iwidth = 1;
        for (lni = i / 10; lni > 0; lni = lni / 10) {
          iwidth++;
        }
        for (lni = iwidth; lni < lwidth; lni++) {
          commonWriter->write(0x0020);
        }
        commonWriter->write(UStr::to_unistr(i));
        commonWriter->write(": ");
      }
      if (useTokens) {
        ParsedLineWriter::tokenWrite(commonWriter, escapedWriter, &docLinkHash,
                                     textLinesStore.getLine(i), baseEditor.getLineRegions(i));
      }
      else if (useMarkup) {
        ParsedLineWriter::markupWrite(commonWriter, escapedWriter, &docLinkHash,
                                      textLinesStore.getLine(i), baseEditor.getLineRegions(i));
      }
      else {
        ParsedLineWriter::htmlRGBWrite(commonWriter, escapedWriter, &docLinkHash,
                                       textLinesStore.getLine(i), baseEditor.getLineRegions(i));
      }
      commonWriter->write("\n");
    }

    if (htmlWrapping && useTokens) {
      commonWriter->write("</pre></body></html>\n");
    }
    else if (htmlWrapping && rd != nullptr) {
      if (useMarkup) {
        commonWriter->write(*TextRegion::cast(rd)->end_text);
      }
      else {
        commonWriter->write("</pre></body></html>\n");
      }
    }

    if (htmlEscaping) {
      delete commonWriter;
    }
    delete escapedWriter;
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
