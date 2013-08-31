#include <colorer/parsers/helpers/FileTypeImpl.h>
#include <unicode/UnicodeTools.h>

FileTypeImpl::FileTypeImpl(HRCParserImpl *hrcParser){
  this->hrcParser = hrcParser;
  protoLoaded = typeLoaded = loadDone = loadBroken = inputSourceLoading = false;
  isPackage = false;
  name = group = description = null;
  baseScheme = null;
  inputSource = null;
}

FileTypeImpl::~FileTypeImpl(){
  delete name;
  delete group;
  delete description;
  delete inputSource;
  int idx;
  for (idx = 0; idx < chooserVector.size(); idx++){
    delete chooserVector.elementAt(idx);
  }
  for (idx = 0; idx < importVector.size(); idx++){
    delete importVector.elementAt(idx);
  }
  for (TypeParameter* s = paramsHash.enumerate(); s!=null; s = paramsHash.next()){
    delete s;
  }
}

Scheme* FileTypeImpl::getBaseScheme() {
  if (!typeLoaded) hrcParser->loadFileType(this);
  return baseScheme;
}

const String* FileTypeImpl::enumerateParameters(int idx) {
  TypeParameter* tp = nullptr;
  if (idx==0){
    tp = paramsHash.enumerate();
  }else{
    tp = paramsHash.next();
  }
  if (tp) return tp->name;
  return nullptr;
}

const String* FileTypeImpl::getParamDescription(const String &name) const{
  TypeParameter* tp = paramsHash.get(&name);
  if (tp) return tp->description;
  return nullptr;
}

const String *FileTypeImpl::getParamValue(const String &name) const{
  TypeParameter* tp = paramsHash.get(&name);
  if (tp){
    if(tp->user_value) return tp->user_value;
    return tp->default_value;
  }
  return nullptr;
}

int FileTypeImpl::getParamValueInt(const String &name, int def) const{
  int val = def;
  UnicodeTools::getNumber(getParamValue(name), &val);
  return val;
}

const String* FileTypeImpl::getParamDefaultValue(const String &name) const{
  TypeParameter* tp = paramsHash.get(&name);
  if (tp) return tp->default_value;
  return nullptr;
}

const String* FileTypeImpl::getParamUserValue(const String &name) const{
  TypeParameter* tp = paramsHash.get(&name);
  if (tp) return tp->user_value;
  return nullptr;
}

TypeParameter* FileTypeImpl::addParam(const String *name){
  TypeParameter* tp = new TypeParameter;
  tp->name = new SString(name);
  paramsHash.put(name, tp);
  return tp;
}

void FileTypeImpl::setParamValue(const String &name, const String *value){
  TypeParameter* tp = paramsHash.get(&name);
  if (tp) {
    delete tp->user_value;
    tp->user_value = new SString(value);
  }
}

void FileTypeImpl::setParamDefaultValue(const String &name, const String *value){
  TypeParameter* tp = paramsHash.get(&name);
  if (tp) {
    delete tp->default_value;
    tp->default_value = new SString(value);
  }
}

void FileTypeImpl::setParamUserValue(const String &name, const String *value){
  setParamValue(name,value);
}

void FileTypeImpl::setParamDescription(const String &name, const String *value){
  TypeParameter* tp = paramsHash.get(&name);
  if (tp) {
    delete tp->description;
    tp->description = new SString(value);
  }
}

void FileTypeImpl::removeParamValue(const String *name){
  paramsHash.remove(name);
}

size_t FileTypeImpl::getParamCount() const{
  return paramsHash.size();
}

size_t FileTypeImpl::getParamUserValueCount() const{
  size_t count=0;
  for (TypeParameter* it = paramsHash.enumerate(); it!=nullptr;it=paramsHash.next()){
    if (it && it->user_value) count++;
  }
  return count;
}

double FileTypeImpl::getPriority(const String *fileName, const String *fileContent) const{
  SMatches match;
  double cur_prior = 0;
  for(int idx = 0; idx < chooserVector.size(); idx++){
    FileTypeChooser *ftc = chooserVector.elementAt(idx);
    if (fileName != null && ftc->isFileName() && ftc->getRE()->parse(fileName, &match))
      cur_prior += ftc->getPrior();
    if (fileContent != null && ftc->isFileContent() && ftc->getRE()->parse(fileContent, &match))
      cur_prior += ftc->getPrior();
  }
  return cur_prior;
}