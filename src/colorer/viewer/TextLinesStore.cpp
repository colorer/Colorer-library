#include <colorer/Exception.h>
#include <colorer/common/Encodings.h>
#include <colorer/io/InputSource.h>
#include <colorer/viewer/TextLinesStore.h>

TextLinesStore::~TextLinesStore()
{
  freeFile();
}

void TextLinesStore::freeFile()
{
  fileName.reset();
  for (auto it : lines) {
    delete it;
  }
  lines.clear();
}

void TextLinesStore::loadFile(const UnicodeString* inFileName, bool tab2spaces)
{
  if (this->fileName) {
    freeFile();
  }

  if (inFileName == nullptr) {
    char line[256];
    while (fgets(line, sizeof(line), stdin) != nullptr) {
      strtok(line, "\r\n");
      lines.push_back(new UnicodeString(line));
      if (tab2spaces) {
        replaceTabs(lines.size() - 1);
      }
    }
  } else {
    this->fileName = std::make_unique<UnicodeString>(*inFileName);
    colorer::InputSource* is = colorer::InputSource::newInstance(inFileName);

    const byte* data;
    try {
      data = is->openStream();
    } catch (InputSourceException&) {
      delete is;
      throw;
    }
    int len = is->length();

    auto file = Encodings::toUnicodeString((char*) data, len);
    auto length = file->length();
    lines.reserve(static_cast<size_t>(length / 30));  // estimate number of lines

    int i = 0;
    int filepos = 0;
    int prevpos = 0;

    while (filepos < length + 1) {
      if (filepos == length || (*file)[filepos] == '\r' || (*file)[filepos] == '\n') {
        lines.push_back(new UnicodeString(*file, prevpos, filepos - prevpos));
        if (tab2spaces) {
          replaceTabs(lines.size() - 1);
        }
        if (filepos + 1 < length && (*file)[filepos] == '\r' && (*file)[filepos + 1] == '\n') {
          filepos++;
        } else if (filepos + 1 < length && (*file)[filepos] == '\n' && (*file)[filepos + 1] == '\r') {
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
  return fileName.get();
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

void TextLinesStore::replaceTabs(size_t lno)
{
  lines.at(lno)->findAndReplace("\t", "    ");
}