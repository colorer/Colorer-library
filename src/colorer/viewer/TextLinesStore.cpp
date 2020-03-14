#include <colorer/Exception.h>
#include <colorer/io/InputSource.h>
#include <colorer/unicode/Encodings.h>
#include <colorer/viewer/TextLinesStore.h>
#include <cstdio>
#include <cstring>

void TextLinesStore::replaceTabs(size_t lno)
{
  lines.at(lno)->findAndReplace("\t", "    ");
}

TextLinesStore::TextLinesStore()
{
  fileName = nullptr;
}

TextLinesStore::~TextLinesStore()
{
  freeFile();
}

void TextLinesStore::freeFile()
{
  delete fileName;
  fileName = nullptr;
  for(auto it:lines){
    delete it;
  }
  lines.clear();
}

void TextLinesStore::loadFile(const UnicodeString* fileName_, const UnicodeString* inputEncoding, bool tab2spaces)
{
  if (this->fileName != nullptr) {
    freeFile();
  }

  if (fileName_ == nullptr) {
    char line[256];
    while (fgets(line, sizeof(line), stdin) != nullptr) {
      strtok(line, "\r\n");
      lines.push_back(new UnicodeString(line));
      if (tab2spaces) {
        replaceTabs(lines.size() - 1);
      }
    }
  } else {
    this->fileName = new UnicodeString(*fileName_);
    colorer::InputSource* is = colorer::InputSource::newInstance(fileName_);

    const byte* data;
    try {
      data = is->openStream();
    } catch (InputSourceException&) {
      delete is;
      throw;
    }
    int len = is->length();

    //TODO codepage
    //int ei = inputEncoding == nullptr ? -1 : Encodings::getEncodingIndex(inputEncoding->getChars());
    //CString file(data, len, ei);
    /*UnicodeString  s((char*)data,len,"utf-8");
    UnicodeString  s1((char*)data,len,"");
    UnicodeString  s2((char*)data,len);
    spdlog::warn("{0}",s);
    spdlog::warn("{0}",s1);
    spdlog::warn("{0}",s2);
    spdlog::warn("{0}",UStr::to_unistr(&file));*/
    UnicodeString  file((char*)data,len);
    int length = file.length();
    lines.reserve(static_cast<size_t>(length / 30)); // estimate number of lines

    int i = 0;
    int filepos = 0;
    int prevpos = 0;
    if (length && file[0] == 0xFEFF) {
      filepos = prevpos = 1;
    }
    while (filepos < length + 1) {
      if (filepos == length || file[filepos] == '\r' || file[filepos] == '\n') {
        lines.push_back(new UnicodeString(file, prevpos, filepos - prevpos));
        if (tab2spaces) {
          replaceTabs(lines.size() - 1);
        }
        if (filepos + 1 < length && file[filepos] == '\r' && file[filepos + 1] == '\n') {
          filepos++;
        } else if (filepos + 1 < length && file[filepos] == '\n' && file[filepos + 1] == '\r') {
          filepos++;
        }
        prevpos = filepos + 1;
        i++;
      }
      filepos++;
    }
    delete is;
  }
}

const UnicodeString* TextLinesStore::getFileName()
{
  return fileName;
}

UnicodeString* TextLinesStore::getLine(size_t lno)
{
  if (lines.size() <= lno) {
    return nullptr;
  }
  return lines[lno];
}

size_t TextLinesStore::getLineCount()
{
  return lines.size();
}


