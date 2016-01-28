#include <colorer/parsers/helpers/FileTypeImpl.h>
#include <unicode/UnicodeTools.h>

FileTypeImpl::FileTypeImpl(HRCParserImpl* hrcParser): name(nullptr), group(nullptr), description(nullptr)
{
  this->hrcParser = hrcParser;
  protoLoaded = type_loaded = loadDone = load_broken = input_source_loading = false;
  isPackage = false;
  baseScheme = nullptr;
  inputSource = nullptr;
}

FileTypeImpl::~FileTypeImpl(){
  delete inputSource;
  for(auto it : chooserVector){
    delete it;
  }
  chooserVector.clear();

  for(auto it: paramsHash){
    delete it.second;
  }
  paramsHash.clear();

  for (auto it : importVector) {
    delete it;
  }
  importVector.clear();
}

Scheme* FileTypeImpl::getBaseScheme() {
  if (!type_loaded) hrcParser->loadFileType(this);
  return baseScheme;
}

std::vector<SString> FileTypeImpl::enumParams() const {
  std::vector<SString> r;
  r.reserve(paramsHash.size());
  for ( auto p = paramsHash.begin(); p != paramsHash.end(); ++p)
  {
	  r.push_back(p->first);
  }
  return r;
}

const String* FileTypeImpl::getParamDescription(const String &name) const{
  auto tp = paramsHash.find(name);
  if (tp != paramsHash.end()) return tp->second->description;
  return nullptr;
}

const String *FileTypeImpl::getParamValue(const String &name) const{
  auto tp = paramsHash.find(name);
  if (tp != paramsHash.end()){
    if(tp->second->user_value) return tp->second->user_value;
    return tp->second->default_value;
  }
  return nullptr;
}

int FileTypeImpl::getParamValueInt(const String &name, int def) const{
  int val = def;
  UnicodeTools::getNumber(getParamValue(name), &val);
  return val;
}

const String* FileTypeImpl::getParamDefaultValue(const String &name) const{
  auto tp = paramsHash.find(name);
  if (tp !=paramsHash.end()) {
    return tp->second->default_value;
  }
  return nullptr;
}

const String* FileTypeImpl::getParamUserValue(const String &name) const{
  auto tp = paramsHash.find(name);
  if (tp !=paramsHash.end()) {
    return tp->second->user_value;
  }
  return nullptr;
}

TypeParameter* FileTypeImpl::addParam(const String *name){
  TypeParameter* tp = new TypeParameter;
  tp->name = new SString(name);
  std::pair<SString, TypeParameter*> pp(name, tp);
  paramsHash.emplace(pp);
  return tp;
}

void FileTypeImpl::setParamValue(const String &name, const String *value){
  auto tp = paramsHash.find(name);
  if (tp != paramsHash.end()) {
    delete tp->second->user_value;
    tp->second->user_value = new SString(value);
  }
}

void FileTypeImpl::setParamDefaultValue(const String &name, const String *value){
  auto tp = paramsHash.find(name);
  if (tp != paramsHash.end()) {
    delete tp->second->default_value;
    tp->second->default_value = new SString(value);
  }
}

void FileTypeImpl::setParamUserValue(const String &name, const String *value){
  setParamValue(name,value);
}

void FileTypeImpl::setParamDescription(const String &name, const String *value){
  auto tp = paramsHash.find(name);
  if (tp != paramsHash.end()) {
    delete tp->second->description;
    tp->second->description = new SString(value);
  }
}

void FileTypeImpl::removeParamValue(const String &name){
  paramsHash.erase(name);
}

size_t FileTypeImpl::getParamCount() const{
  return paramsHash.size();
}

size_t FileTypeImpl::getParamUserValueCount() const{
  size_t count=0;
  for ( auto it = paramsHash.begin(); it != paramsHash.end(); ++it){
    if (it->second->user_value) count++;
  }
  return count;
}

double FileTypeImpl::getPriority(const String *fileName, const String *fileContent) const{
  SMatches match;
  double cur_prior = 0;
  for(size_t idx = 0; idx < chooserVector.size(); idx++){
    FileTypeChooser *ftc = chooserVector.at(idx);
    if (fileName != nullptr && ftc->isFileName() && ftc->getRE()->parse(fileName, &match))
      cur_prior += ftc->getPrior();
    if (fileContent != nullptr && ftc->isFileContent() && ftc->getRE()->parse(fileContent, &match))
      cur_prior += ftc->getPrior();
  }
  return cur_prior;
}