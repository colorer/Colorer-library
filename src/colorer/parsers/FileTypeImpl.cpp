#include <colorer/parsers/FileTypeImpl.h>
#include <colorer/unicode/UnicodeTools.h>

#include <memory>
#include <colorer/unicode/SString.h>

FileTypeImpl::FileTypeImpl(HRCParserImpl* hrcParser): name(nullptr), group(nullptr), description(nullptr)
{
  this->hrcParser = hrcParser;
  protoLoaded = type_loaded = loadDone = load_broken = input_source_loading = false;
  isPackage = false;
  baseScheme = nullptr;
  inputSource = nullptr;
}

FileTypeImpl::~FileTypeImpl(){
  for(auto it : chooserVector){
    delete it;
  }
  chooserVector.clear();

  for(const auto& it: paramsHash){
    delete it.second;
  }
  paramsHash.clear();

  importVector.clear();
}

Scheme* FileTypeImpl::getBaseScheme() {
  if (!type_loaded) hrcParser->loadFileType(this);
  return baseScheme;
}

std::vector<UnicodeString> FileTypeImpl::enumParams() const {
  std::vector<UnicodeString> r;
  r.reserve(paramsHash.size());
  for (const auto & p : paramsHash)
  {
	  r.push_back(p.first);
  }
  return r;
}

const UnicodeString* FileTypeImpl::getParamDescription(const UnicodeString &name) const{
  auto tp = paramsHash.find(name);
  if (tp != paramsHash.end()) return tp->second->description.get();
  return nullptr;
}

const UnicodeString *FileTypeImpl::getParamValue(const UnicodeString &name) const{
  auto tp = paramsHash.find(name);
  if (tp != paramsHash.end()){
    if(tp->second->user_value) return tp->second->user_value.get();
    return tp->second->default_value.get();
  }
  return nullptr;
}

int FileTypeImpl::getParamValueInt(const UnicodeString &name, int def) const{
  int val = def;
  UnicodeTools::getNumber(&UStr::to_string(getParamValue(name)), &val);
  return val;
}

const UnicodeString* FileTypeImpl::getParamDefaultValue(const UnicodeString &name) const{
  auto tp = paramsHash.find(name);
  if (tp !=paramsHash.end()) {
    return tp->second->default_value.get();
  }
  return nullptr;
}

const UnicodeString* FileTypeImpl::getParamUserValue(const UnicodeString &name) const{
  auto tp = paramsHash.find(name);
  if (tp !=paramsHash.end()) {
    return tp->second->user_value.get();
  }
  return nullptr;
}

TypeParameter* FileTypeImpl::addParam(const UnicodeString *name){
  auto* tp = new TypeParameter;
  tp->name = std::make_unique<UnicodeString>(*name);
  std::pair<UnicodeString, TypeParameter*> pp(*name, tp);
  paramsHash.emplace(pp);
  return tp;
}

void FileTypeImpl::setParamValue(const UnicodeString &name, const UnicodeString *value){
  auto tp = paramsHash.find(name);
  if (tp != paramsHash.end()) {
    tp->second->user_value = std::make_unique<UnicodeString>(*value);
  }
}

void FileTypeImpl::setParamDefaultValue(const UnicodeString &name, const UnicodeString *value){
  auto tp = paramsHash.find(name);
  if (tp != paramsHash.end()) {
    tp->second->default_value = std::make_unique<UnicodeString>(*value);
  }
}

void FileTypeImpl::setParamUserValue(const UnicodeString &name, const UnicodeString *value){
  setParamValue(name,value);
}

void FileTypeImpl::setParamDescription(const UnicodeString &name, const UnicodeString *value){
  auto tp = paramsHash.find(name);
  if (tp != paramsHash.end()) {
    tp->second->description = std::make_unique<UnicodeString>(*value);
  }
}

void FileTypeImpl::removeParamValue(const UnicodeString &name){
  paramsHash.erase(name);
}

size_t FileTypeImpl::getParamCount() const{
  return paramsHash.size();
}

size_t FileTypeImpl::getParamUserValueCount() const{
  size_t count=0;
  for (const auto & it : paramsHash){
    if (it.second->user_value) count++;
  }
  return count;
}

double FileTypeImpl::getPriority(const UnicodeString *fileName, const UnicodeString *fileContent) const{
  SMatches match;
  double cur_prior = 0;
  for(auto ftc : chooserVector){
    if (fileName != nullptr && ftc->isFileName() && ftc->getRE()->parse(fileName, &match))
      cur_prior += ftc->getPriority();
    if (fileContent != nullptr && ftc->isFileContent() && ftc->getRE()->parse(fileContent, &match))
      cur_prior += ftc->getPriority();
  }
  return cur_prior;
}

