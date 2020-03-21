#include <colorer/editor/BaseEditor.h>
#include <colorer/common/UnicodeLogger.h>
#include <colorer/parsers/TextParserImpl.h>

#define IDLE_PARSE(time) (100+(time)*4)

const int CHOOSE_STR = 4;
const int CHOOSE_LEN = 200 * CHOOSE_STR;

BaseEditor::BaseEditor(ParserFactory* parserFactory_, LineSource* lineSource_)
{
  if (parserFactory_ == nullptr || lineSource_ == nullptr) {
    throw Exception("Bad BaseEditor constructor parameters");
  }
  parserFactory = parserFactory_;
  lineSource = lineSource_;

  hrcParser = parserFactory->getHRCParser();
  textParser = parserFactory->createTextParser();

  textParser->setRegionHandler(this);
  textParser->setLineSource(lineSource);

  lrSupport = nullptr;

  invalidLine = 0;
  changedLine = 0;
  backParse = -1;
  lineCount = 0;
  wStart = 0;
  wSize = 20;
  lrSize = wSize * 3;
  internalRM = false;
  regionMapper = nullptr;
  regionCompact = false;
  currentFileType = nullptr;

  breakParse = false;
  validationProcess = false;

  UnicodeString def_text = UnicodeString("def:Text");
  UnicodeString def_syntax = UnicodeString("def:Syntax");
  UnicodeString def_special = UnicodeString("def:Special");
  UnicodeString def_pstart = UnicodeString("def:PairStart");
  UnicodeString def_pend = UnicodeString("def:PairEnd");
  def_Text = hrcParser->getRegion(&def_text);
  def_Syntax = hrcParser->getRegion(&def_syntax);
  def_Special = hrcParser->getRegion(&def_special);
  def_PairStart = hrcParser->getRegion(&def_pstart);
  def_PairEnd = hrcParser->getRegion(&def_pend);

  setRegionCompact(regionCompact);

  rd_def_Text = rd_def_HorzCross = rd_def_VertCross = nullptr;
}

BaseEditor::~BaseEditor()
{
  textParser->breakParse();
  breakParse = true;
  while (validationProcess){}; /// @todo wait until validation is finished
  if (internalRM) {
    delete regionMapper;
  }
  delete lrSupport;
  delete textParser;
}

void BaseEditor::setRegionCompact(bool compact)
{
  if (!lrSupport || regionCompact != compact) {
    regionCompact = compact;
    remapLRS(true);
  }
}

void BaseEditor::setRegionMapper(RegionMapper* rs)
{
  if (internalRM) {
    delete regionMapper;
  }
  regionMapper = rs;
  internalRM = false;
  remapLRS(false);
}

void BaseEditor::setRegionMapper(const UnicodeString* hrdClass, const UnicodeString* hrdName)
{
  if (internalRM) {
    delete regionMapper;
  }
  regionMapper = parserFactory->createStyledMapper(hrdClass, hrdName);
  internalRM = true;
  remapLRS(false);
}

void BaseEditor::remapLRS(bool recreate)
{
  if (recreate || lrSupport == nullptr) {
    delete lrSupport;
    if (regionCompact) {
      lrSupport = new LineRegionsCompactSupport();
    } else {
      lrSupport = new LineRegionsSupport();
    }
    lrSupport->resize(lrSize);
    lrSupport->clear();
  }
  lrSupport->setRegionMapper(regionMapper);
  lrSupport->setSpecialRegion(def_Special);
  invalidLine = 0;
  rd_def_Text = rd_def_HorzCross = rd_def_VertCross = nullptr;
  if (regionMapper != nullptr) {
    rd_def_Text = regionMapper->getRegionDefine("def:Text");
    rd_def_HorzCross = regionMapper->getRegionDefine("def:HorzCross");
    rd_def_VertCross = regionMapper->getRegionDefine("def:VertCross");
  }
}

void BaseEditor::setFileType(FileType* ftype)
{
  spdlog::debug("[BaseEditor] setFileType: {0}", *ftype->getName());
  currentFileType = ftype;
  hrcParser->loadFileType(ftype);
  textParser->setFileType(currentFileType);
  invalidLine = 0;
}

FileType* BaseEditor::setFileType(const UnicodeString& fileType)
{
  currentFileType = hrcParser->getFileType(&fileType);
  setFileType(currentFileType);
  return currentFileType;
}


FileType* BaseEditor::chooseFileTypeCh(const UnicodeString* fileName, int chooseStr, int chooseLen)
{
  UnicodeString textStart;
  int totalLength = 0;
  for (int i = 0; i < chooseStr; i++) {
    UnicodeString* iLine = lineSource->getLine(i);
    if (iLine == nullptr) {
      break;
    }
    textStart.append(*iLine);
    textStart.append("\n");
    totalLength += iLine->length();
    if (totalLength > chooseLen) {
      break;
    }
  }
  currentFileType = hrcParser->chooseFileType(fileName, &textStart);

  int chooseStrNext = currentFileType->getParamValueInt("firstlines", chooseStr);
  int chooseLenNext = currentFileType->getParamValueInt("firstlinebytes", chooseLen);

  if (chooseStrNext != chooseStr || chooseLenNext != chooseLen) {
    currentFileType = chooseFileTypeCh(fileName, chooseStrNext, chooseLenNext);
  }
  return currentFileType;
}

FileType* BaseEditor::chooseFileType(const UnicodeString* fileName)
{
  if (lineSource == nullptr) {
    currentFileType = hrcParser->chooseFileType(fileName, nullptr);
  } else {
    int chooseStr = CHOOSE_STR, chooseLen = CHOOSE_LEN;

    UnicodeString ds_def = UnicodeString("default");
    FileType* def = hrcParser->getFileType(&ds_def);
    if (def) {
      chooseStr = def->getParamValueInt("firstlines", chooseStr);
      chooseLen = def->getParamValueInt("firstlinebytes", chooseLen);
    }

    currentFileType = chooseFileTypeCh(fileName, chooseStr, chooseLen);
  }
  setFileType(currentFileType);
  return currentFileType;
}


FileType* BaseEditor::getFileType()
{
  return currentFileType;
}

void BaseEditor::setBackParse(int backParse)
{
  this->backParse = backParse;
}

void BaseEditor::addRegionHandler(RegionHandler* rh)
{
  regionHandlers.push_back(rh);
}

void BaseEditor::removeRegionHandler(RegionHandler* rh)
{
  for (auto ft = regionHandlers.begin(); ft != regionHandlers.end(); ++ft) {
    if (*ft == rh) {
      regionHandlers.erase(ft);
      break;
    }
  }
}

void BaseEditor::addEditorListener(EditorListener* el)
{
  editorListeners.push_back(el);
}

void BaseEditor::removeEditorListener(EditorListener* el)
{
  for (auto ft = editorListeners.begin(); ft != editorListeners.end(); ++ft) {
    if (*ft == el) {
      editorListeners.erase(ft);
      break;
    }
  }
}


PairMatch* BaseEditor::getPairMatch(int lineNo, int linePos)
{
  LineRegion* lrStart = getLineRegions(lineNo);
  if (lrStart == nullptr) {
    return nullptr;
  }
  LineRegion* pair = nullptr;
  for (LineRegion* l1 = lrStart; l1; l1 = l1->next) {
    if ((l1->region->hasParent(def_PairStart) ||
         l1->region->hasParent(def_PairEnd)) &&
        linePos >= l1->start && linePos <= l1->end) {
      pair = l1;
    }
  }
  if (pair != nullptr) {
    auto* pm = new PairMatch(pair, lineNo, pair->region->hasParent(def_PairStart));
    pm->setStart(pair);
    return pm;
  }
  return nullptr;
}

PairMatch* BaseEditor::getEnwrappedPairMatch(int lineNo, int pos)
{
  return nullptr;
}

void BaseEditor::releasePairMatch(PairMatch* pm)
{
  delete pm;
}

PairMatch* BaseEditor::searchLocalPair(int lineNo, int pos)
{
  int lno;
  int end_line = getLastVisibleLine();
  PairMatch* pm = getPairMatch(lineNo, pos);
  if (pm == nullptr) {
    return nullptr;
  }

  lno = pm->sline;

  LineRegion* pair = pm->getStartRef();
  LineRegion* slr = getLineRegions(lno);
  while (true) {
    if (pm->pairBalance > 0) {
      pair = pair->next;
      while (pair == nullptr) {
        lno++;
        if (lno > end_line) {
          break;
        }
        pair = getLineRegions(lno);
      }
      if (lno > end_line) {
        break;
      }
    } else {
      if (pair->prev == slr->prev) { // first region
        lno--;
        if (lno < wStart) {
          break;
        }
        slr = getLineRegions(lno);
        pair = slr;
      }
      if (lno < wStart) {
        break;
      }
      pair = pair->prev;
    }
    if (pair->region->hasParent(def_PairStart)) {
      pm->pairBalance++;
    }
    if (pair->region->hasParent(def_PairEnd)) {
      pm->pairBalance--;
    }
    if (pm->pairBalance == 0) {
      break;
    }
  }
  if (pm->pairBalance == 0) {
    pm->eline = lno;
    pm->setEnd(pair);
  }
  return pm;
}

PairMatch* BaseEditor::searchGlobalPair(int lineNo, int pos)
{
  int lno;
  int end_line = lineCount;
  PairMatch* pm = getPairMatch(lineNo, pos);
  if (pm == nullptr) {
    return nullptr;
  }

  lno = pm->sline;

  LineRegion* pair = pm->getStartRef();
  LineRegion* slr = getLineRegions(lno);
  while (true) {
    if (pm->pairBalance > 0) {
      pair = pair->next;
      while (pair == nullptr) {
        lno++;
        if (lno > end_line) {
          break;
        }
        pair = getLineRegions(lno);
      }
      if (lno > end_line) {
        break;
      }
    } else {
      if (pair->prev == slr->prev) { // first region
        lno--;
        if (lno < 0) {
          break;
        }
        slr = getLineRegions(lno);
        pair = slr;
      }
      if (lno < 0) {
        break;
      }
      pair = pair->prev;
    }
    if (pair->region->hasParent(def_PairStart)) {
      pm->pairBalance++;
    }
    if (pair->region->hasParent(def_PairEnd)) {
      pm->pairBalance--;
    }
    if (pm->pairBalance == 0) {
      break;
    }
  }
  if (pm->pairBalance == 0) {
    pm->eline = lno;
    pm->setEnd(pair);
  }
  return pm;
}


LineRegion* BaseEditor::getLineRegions(int lno)
{
  /*
   * Backparse value check
   */
  if (backParse > 0 && lno - invalidLine > backParse) {
    return nullptr;
  }
  validate(lno, true);
  return lrSupport->getLineRegions(lno);
}

void BaseEditor::modifyEvent(int topLine)
{
  spdlog::debug("[BaseEditor] modifyEvent: {0}", topLine);
  if (invalidLine > topLine) {
    invalidLine = topLine;
    for (auto & editorListener : editorListeners) {
      editorListener->modifyEvent(topLine);
    }
  }
}

void BaseEditor::modifyLineEvent(int line)
{
  if (invalidLine > line) {
    invalidLine = line;
  }
  // changedLine = topLine;!!!
}

void BaseEditor::visibleTextEvent(int wStart_, int wSize_)
{
  spdlog::debug("[BaseEditor] visibleTextEvent: {0}-{1}", wStart_, wSize_);
  wStart = wStart_;
  wSize = wSize_;
}

void BaseEditor::lineCountEvent(int newLineCount)
{
  spdlog::debug("[BaseEditor] lineCountEvent: {0}", newLineCount);
  lineCount = newLineCount;
}


inline int BaseEditor::getLastVisibleLine()
{
  int r1 = (wStart + wSize);
  int r2 = lineCount;
  return ((r1 > r2) ? r2 : r1) - 1;
}

void BaseEditor::validate(int lno, bool rebuildRegions)
{
  int parseFrom, parseTo;
  bool layoutChanged = false;
  TextParser::TextParseMode tpmode = TextParser::TPM_CACHE_READ;

  if (lno == -1 || lno > lineCount) {
    lno = lineCount - 1;
  }

  size_t firstLine = lrSupport->getFirstLine();
  parseFrom = parseTo = (wStart + wSize);

  /*
   * Calculate changes, required by new screen position, if any
   */
  if (lrSize != wSize * 2) {
    lrSize = wSize * 2;
    lrSupport->resize(lrSize);
    lrSupport->clear();
    // Regions were dropped
    layoutChanged = true;
    spdlog::debug("[BaseEditor] lrSize != wSize*2");
  }

  /* Fixes window position according to line number */
  if (lno < wStart || lno > wStart + wSize) {
    wStart = lno;
    //if enable, introduces heavy delays on pair searching
    //layoutChanged = true;
  }

  if (layoutChanged || wStart < (int)firstLine || wStart + wSize > (int)firstLine + lrSize) {
    /*
     * visible area is shifted and line regions
     * should be rearranged according to
     */
    int newFirstLine = (wStart / wSize) * wSize;
    parseFrom = newFirstLine;
    parseTo   = newFirstLine + lrSize;
    /*
     * Change LineRegions parameters only in case
     * of validate-for-usage request.
     */
    if (rebuildRegions) {
      lrSupport->setFirstLine(newFirstLine);
    }
    /* Save time - already has the info in line cache */
    if (!layoutChanged && (int)firstLine - newFirstLine == wSize) {
      parseTo -= wSize - 1;
    }
    firstLine = newFirstLine;
    layoutChanged = true;
    spdlog::debug("[BaseEditor] newFirstLine={0}, parseFrom={1}, parseTo={2}", firstLine, parseFrom, parseTo);
  }

  if (!layoutChanged) {
    /* Text modification only event */
    if (invalidLine <= parseTo) {
      parseFrom = invalidLine;
      tpmode = TextParser::TPM_CACHE_UPDATE;
    }
  }

  /* Text modification general ajustment */
  if (invalidLine <= parseFrom) {
    parseFrom = invalidLine;
    tpmode = TextParser::TPM_CACHE_UPDATE;
  }

  if (parseTo > lineCount) {
    parseTo = lineCount;
  }

  /* Runs parser */
  if (parseTo - parseFrom > 0) {

    spdlog::debug("[BaseEditor] validate:parse:{0}-{1}, {2}", parseFrom, parseTo, tpmode == TextParser::TPM_CACHE_READ ? "READ" : "UPDATE");
    int stopLine = textParser->parse(parseFrom, parseTo - parseFrom, tpmode);

    if (tpmode == TextParser::TPM_CACHE_UPDATE) {
      invalidLine = stopLine + 1;
    }
    spdlog::debug("[BaseEditor] validate:parsed: invalidLine={0}", invalidLine);
  }
}

void BaseEditor::idleJob(int time)
{
  if (invalidLine < lineCount) {
    if (time < 0) {
      time = 0;
    }
    if (time > 100) {
      time = 100;
    }
    validate(invalidLine + IDLE_PARSE(time), false);
  }
}

void BaseEditor::startParsing(size_t lno)
{
  lrSupport->startParsing(lno);
  for (auto & regionHandler : regionHandlers) {
    regionHandler->startParsing(lno);
  }
}

void BaseEditor::endParsing(size_t lno)
{
  lrSupport->endParsing(lno);
  for (auto & regionHandler : regionHandlers) {
    regionHandler->endParsing(lno);
  }
}

void BaseEditor::clearLine(size_t lno, UnicodeString* line)
{
  lrSupport->clearLine(lno, line);
  for (auto & regionHandler : regionHandlers) {
    regionHandler->clearLine(lno, line);
  }
}

void BaseEditor::addRegion(size_t lno, UnicodeString* line, int sx, int ex, const Region* region)
{
  lrSupport->addRegion(lno, line, sx, ex, region);
  for (auto & regionHandler : regionHandlers) {
    regionHandler->addRegion(lno, line, sx, ex, region);
  }
}

void BaseEditor::enterScheme(size_t lno, UnicodeString* line, int sx, int ex, const Region* region, const Scheme* scheme)
{
  lrSupport->enterScheme(lno, line, sx, ex, region, scheme);
  for (auto & regionHandler : regionHandlers) {
    regionHandler->enterScheme(lno, line, sx, ex, region, scheme);
  }
}

void BaseEditor::leaveScheme(size_t lno, UnicodeString* line, int sx, int ex, const Region* region, const Scheme* scheme)
{
  lrSupport->leaveScheme(lno, line, sx, ex, region, scheme);
  for (auto & regionHandler : regionHandlers) {
    regionHandler->leaveScheme(lno, line, sx, ex, region, scheme);
  }
}

bool BaseEditor::haveInvalidLine()
{
  return invalidLine < lineCount;
}



