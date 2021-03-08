#include <colorer/viewer/ParsedLineWriter.h>

void ParsedLineWriter::tokenWrite(Writer* markupWriter, Writer* textWriter, std::unordered_map<UnicodeString, UnicodeString*>* /*docLinkHash*/,
                                  UnicodeString* line, LineRegion* lineRegions)
{
  int pos = 0;
  for (LineRegion* l1 = lineRegions; l1; l1 = l1->next) {
    if (l1->special || l1->region == nullptr)
      continue;
    if (l1->start == l1->end)
      continue;
    int end = l1->end;
    if (end == -1)
      end = line->length();
    if (l1->start > pos) {
      textWriter->write(line, pos, l1->start - pos);
      pos = l1->start;
    }
    markupWriter->write("<span class='");

    const Region* region = l1->region;
    while (region != nullptr) {
      UnicodeString token = UnicodeString(*region->getName()).findAndReplace(":", "-").findAndReplace(".", "-");
      markupWriter->write(token);
      region = region->getParent();
      if (region != nullptr) {
        markupWriter->write(' ');
      }
    }

    markupWriter->write("'>");
    textWriter->write(line, pos, end - l1->start);
    markupWriter->write("</span>");
    pos += end - l1->start;
  }
  if (pos < line->length()) {
    textWriter->write(line, pos, line->length() - pos);
  }
}

void ParsedLineWriter::markupWrite(Writer* markupWriter, Writer* textWriter, std::unordered_map<UnicodeString, UnicodeString*>* /*docLinkHash*/,
                                   UnicodeString* line, LineRegion* lineRegions)
{
  int pos = 0;
  for (LineRegion* l1 = lineRegions; l1; l1 = l1->next) {
    if (l1->special || l1->rdef == nullptr)
      continue;
    if (l1->start == l1->end)
      continue;
    int end = l1->end;
    if (end == -1)
      end = line->length();
    if (l1->start > pos) {
      textWriter->write(line, pos, l1->start - pos);
      pos = l1->start;
    }
    if (l1->texted()->start_back != nullptr)
      markupWriter->write(*l1->texted()->start_back);
    if (l1->texted()->start_text != nullptr)
      markupWriter->write(*l1->texted()->start_text);
    textWriter->write(line, pos, end - l1->start);
    if (l1->texted()->end_text != nullptr)
      markupWriter->write(*l1->texted()->end_text);
    if (l1->texted()->end_back != nullptr)
      markupWriter->write(*l1->texted()->end_back);
    pos += end - l1->start;
  }
  if (pos < line->length()) {
    textWriter->write(line, pos, line->length() - pos);
  }
}

void ParsedLineWriter::htmlRGBWrite(Writer* markupWriter, Writer* textWriter, std::unordered_map<UnicodeString, UnicodeString*>* docLinkHash,
                                    UnicodeString* line, LineRegion* lineRegions)
{
  int pos = 0;
  for (LineRegion* l1 = lineRegions; l1; l1 = l1->next) {
    if (l1->special || l1->rdef == nullptr)
      continue;
    if (l1->start == l1->end)
      continue;
    int end = l1->end;
    if (end == -1)
      end = line->length();
    if (l1->start > pos) {
      textWriter->write(line, pos, l1->start - pos);
      pos = l1->start;
    }
    if (!docLinkHash->empty())
      writeHref(markupWriter, docLinkHash, l1->scheme, UnicodeString(*line, pos, end - l1->start), true);
    writeStart(markupWriter, l1->styled());
    textWriter->write(line, pos, end - l1->start);
    writeEnd(markupWriter, l1->styled());
    if (!docLinkHash->empty())
      writeHref(markupWriter, docLinkHash, l1->scheme, UnicodeString(*line, pos, end - l1->start), false);
    pos += end - l1->start;
  }
  if (pos < line->length()) {
    textWriter->write(line, pos, line->length() - pos);
  }
}

void ParsedLineWriter::writeStyle(Writer* writer, const StyledRegion* lr)
{
  static char span[256];
  constexpr auto size_span = std::size(span);
  int cp = 0;
  if (lr->isForeSet)
    cp += snprintf(span, size_span, "color:#%.6x; ", lr->fore);
  if (lr->isBackSet)
    cp += snprintf(span + cp, size_span - cp, "background:#%.6x; ", lr->back);
  if (lr->style & StyledRegion::RD_BOLD)
    cp += snprintf(span + cp, size_span - cp, "font-weight:bold; ");
  if (lr->style & StyledRegion::RD_ITALIC)
    cp += snprintf(span + cp, size_span - cp, "font-style:italic; ");
  if (lr->style & StyledRegion::RD_UNDERLINE)
    cp += snprintf(span + cp, size_span - cp, "text-decoration:underline; ");
  if (lr->style & StyledRegion::RD_STRIKEOUT)
    cp += snprintf(span + cp, size_span - cp, "text-decoration:strikeout; ");
  if (cp > 0)
    writer->write(span);
}

void ParsedLineWriter::writeStart(Writer* writer, const StyledRegion* lr)
{
  if (!lr->isForeSet && !lr->isBackSet)
    return;
  writer->write("<span style='");
  writeStyle(writer, lr);
  writer->write("'>");
}

void ParsedLineWriter::writeEnd(Writer* writer, const StyledRegion* lr)
{
  if (!lr->isForeSet && !lr->isBackSet)
    return;
  writer->write("</span>");
}

void ParsedLineWriter::writeHref(Writer* writer, std::unordered_map<UnicodeString, UnicodeString*>* docLinkHash, const Scheme* scheme,
                                 const UnicodeString& token, bool start)
{
  UnicodeString* url = nullptr;
  if (scheme != nullptr) {
    auto it_url = docLinkHash->find(UnicodeString(token).append("--").append(*scheme->getName()));
    if (it_url != docLinkHash->end()) {
      url = it_url->second;
    }
  }
  if (url == nullptr) {
    auto it_url = docLinkHash->find(token);
    if (it_url != docLinkHash->end()) {
      url = it_url->second;
    }
  }
  if (url != nullptr) {
    if (start)
      writer->write("<a href='" + *url + "'>");
    else
      writer->write("</a>");
  }
}