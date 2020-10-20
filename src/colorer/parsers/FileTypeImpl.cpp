#include <colorer/common/UStr.h>
#include <colorer/parsers/FileTypeImpl.h>

FileType::Impl::Impl() : name(nullptr), group(nullptr), description(nullptr)
{
  protoLoaded = type_loaded = loadDone = load_broken = input_source_loading = false;
  isPackage = false;
  baseScheme = nullptr;
  inputSource = nullptr;
}

FileType::Impl::~Impl()
{
  for (auto it : chooserVector) {
    delete it;
  }
  chooserVector.clear();

  for (const auto& it : paramsHash) {
    delete it.second;
  }
  paramsHash.clear();

  importVector.clear();
}

Scheme* FileType::Impl::getBaseScheme()
{
  return baseScheme;
}

std::vector<UnicodeString> FileType::Impl::enumParams() const
{
  std::vector<UnicodeString> r;
  r.reserve(paramsHash.size());
  for (const auto& p : paramsHash) {
    r.push_back(p.first);
  }
  return r;
}

const UnicodeString* FileType::Impl::getParamDescription(const UnicodeString& name_) const
{
  auto tp = paramsHash.find(name_);
  if (tp != paramsHash.end())
    return tp->second->description.get();
  return nullptr;
}

const UnicodeString* FileType::Impl::getParamValue(const UnicodeString& name_) const
{
  auto tp = paramsHash.find(name_);
  if (tp != paramsHash.end()) {
    if (tp->second->user_value)
      return tp->second->user_value.get();
    return tp->second->default_value.get();
  }
  return nullptr;
}

int FileType::Impl::getParamValueInt(const UnicodeString& name_, int def) const
{
  int val = def;
  auto param_value = getParamValue(name_);
  if (param_value) {
    auto param_str = UStr::to_stdstr(param_value);
    try {
      val = std::stoi(param_str, nullptr);
    } catch (std::exception&) {
      spdlog::error("Error parse param {0} with value {1} to integer number", name_, param_str);
    }
  }
  return val;
}

const UnicodeString* FileType::Impl::getParamDefaultValue(const UnicodeString& name_) const
{
  auto tp = paramsHash.find(name_);
  if (tp != paramsHash.end()) {
    return tp->second->default_value.get();
  }
  return nullptr;
}

const UnicodeString* FileType::Impl::getParamUserValue(const UnicodeString& name_) const
{
  auto tp = paramsHash.find(name_);
  if (tp != paramsHash.end()) {
    return tp->second->user_value.get();
  }
  return nullptr;
}

TypeParameter* FileType::Impl::addParam(const UnicodeString* name_)
{
  auto* tp = new TypeParameter;
  tp->name = std::make_unique<UnicodeString>(*name_);
  std::pair<UnicodeString, TypeParameter*> pp(*name_, tp);
  paramsHash.emplace(pp);
  return tp;
}

void FileType::Impl::setParamValue(const UnicodeString& name_, const UnicodeString* value)
{
  auto tp = paramsHash.find(name_);
  if (tp != paramsHash.end()) {
    tp->second->user_value = std::make_unique<UnicodeString>(*value);
  }
}

void FileType::Impl::setParamDefaultValue(const UnicodeString& name_, const UnicodeString* value)
{
  auto tp = paramsHash.find(name_);
  if (tp != paramsHash.end()) {
    tp->second->default_value = std::make_unique<UnicodeString>(*value);
  }
}

void FileType::Impl::setParamUserValue(const UnicodeString& name_, const UnicodeString* value)
{
  setParamValue(name_, value);
}

void FileType::Impl::setParamDescription(const UnicodeString& name_, const UnicodeString* value)
{
  auto tp = paramsHash.find(name_);
  if (tp != paramsHash.end()) {
    tp->second->description = std::make_unique<UnicodeString>(*value);
  }
}

void FileType::Impl::removeParamValue(const UnicodeString& name_)
{
  paramsHash.erase(name_);
}

size_t FileType::Impl::getParamCount() const
{
  return paramsHash.size();
}

size_t FileType::Impl::getParamUserValueCount() const
{
  size_t count = 0;
  for (const auto& it : paramsHash) {
    if (it.second->user_value)
      count++;
  }
  return count;
}

double FileType::Impl::getPriority(const UnicodeString* fileName, const UnicodeString* fileContent) const
{
  SMatches match {};
  double cur_prior = 0;
  for (auto ftc : chooserVector) {
    if (fileName != nullptr && ftc->isFileName() && ftc->getRE()->parse(fileName, &match))
      cur_prior += ftc->getPriority();
    if (fileContent != nullptr && ftc->isFileContent() && ftc->getRE()->parse(fileContent, &match))
      cur_prior += ftc->getPriority();
  }
  return cur_prior;
}
