#include "colorer/parsers/FileTypeImpl.h"
#include "colorer/common/UStr.h"

const UnicodeString* FileType::Impl::getName() const
{
  return name.get();
}

const UnicodeString* FileType::Impl::getGroup() const
{
  return group.get();
}

const UnicodeString* FileType::Impl::getDescription() const
{
  return description.get();
}

void FileType::Impl::setName(const UnicodeString* param_name)
{
  name = std::make_unique<UnicodeString>(*param_name);
}

void FileType::Impl::setGroup(const UnicodeString* group_name)
{
  group = std::make_unique<UnicodeString>(*group_name);
}

void FileType::Impl::setDescription(const UnicodeString* description_)
{
  description = std::make_unique<UnicodeString>(*description_);
}

Scheme* FileType::Impl::getBaseScheme() const
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

const UnicodeString* FileType::Impl::getParamDescription(const UnicodeString& param_name) const
{
  auto tp = paramsHash.find(param_name);
  if (tp != paramsHash.end()) {
    return tp->second->description.get();
  }
  return nullptr;
}

const UnicodeString* FileType::Impl::getParamValue(const UnicodeString& param_name) const
{
  auto tp = paramsHash.find(param_name);
  if (tp != paramsHash.end()) {
    if (tp->second->user_value) {
      return tp->second->user_value.get();
    }
    return tp->second->value.get();
  }
  return nullptr;
}

int FileType::Impl::getParamValueInt(const UnicodeString& param_name, int def) const
{
  int val = def;
  auto param_value = getParamValue(param_name);
  if (param_value && param_value->length() > 0) {
    auto param_str = UStr::to_stdstr(param_value);
    try {
      val = std::stoi(param_str, nullptr);
    } catch (std::exception&) {
      spdlog::error("Error parse param {0} with value {1} to integer number", param_name,
                    param_str);
    }
  }
  return val;
}

const UnicodeString* FileType::Impl::getParamDefaultValue(const UnicodeString& param_name) const
{
  auto tp = paramsHash.find(param_name);
  if (tp != paramsHash.end()) {
    return tp->second->value.get();
  }
  return nullptr;
}

const UnicodeString* FileType::Impl::getParamUserValue(const UnicodeString& param_name) const
{
  auto tp = paramsHash.find(param_name);
  if (tp != paramsHash.end()) {
    return tp->second->user_value.get();
  }
  return nullptr;
}

TypeParameter& FileType::Impl::addParam(const UnicodeString& param_name, const UnicodeString& value)
{
  auto tp = std::make_unique<TypeParameter>(param_name, value);
  auto [it, status] = paramsHash.emplace(std::make_pair(param_name, std::move(tp)));
  return *it->second;
}

void FileType::Impl::setParamValue(const UnicodeString& param_name, const UnicodeString* value)
{
  auto tp = paramsHash.find(param_name);
  if (tp != paramsHash.end()) {
    if (value) {
      tp->second->user_value = std::make_unique<UnicodeString>(*value);
    }
    else {
      tp->second->user_value.reset();
    }
  }
  else {
    throw FileTypeException("Don`t set value " + *value + " for parameter \"" + param_name +
                            "\". Parameter not exists.");
  }
}

void FileType::Impl::setParamDefaultValue(const UnicodeString& param_name,
                                          const UnicodeString* value)
{
  auto tp = paramsHash.find(param_name);
  if (tp != paramsHash.end()) {
    if (value) {
      tp->second->value = std::make_unique<UnicodeString>(*value);
    }
    else {
      throw FileTypeException("Don`t set null value for parameter \"" + param_name + "\"");
    }
  }
  else {
    throw FileTypeException("Don`t set value " + *value + " for parameter \"" + param_name +
                            "\". Parameter not exists.");
  }
}

void FileType::Impl::setParamUserValue(const UnicodeString& param_name, const UnicodeString* value)
{
  setParamValue(param_name, value);
}

void FileType::Impl::setParamDescription(const UnicodeString& param_name,
                                         const UnicodeString* t_description)
{
  auto tp = paramsHash.find(param_name);
  if (tp != paramsHash.end()) {
    tp->second->description = std::make_unique<UnicodeString>(*t_description);
  }
  else {
    throw FileTypeException("Don`t set value " + *t_description +
                            " for description of parameter \"" + param_name +
                            "\". Parameter not exists.");
  }
}

size_t FileType::Impl::getParamCount() const
{
  return paramsHash.size();
}

double FileType::Impl::getPriority(const UnicodeString* fileName,
                                   const UnicodeString* fileContent) const
{
  SMatches match {};
  double cur_prior {0};
  for (auto const& ftc : chooserVector) {
    if (ftc->isFileName() && fileName != nullptr && ftc->getRE()->parse(fileName, &match)) {
      cur_prior += ftc->getPriority();
    }
    else if (ftc->isFileContent() && fileContent != nullptr &&
             ftc->getRE()->parse(fileContent, &match)) {
      cur_prior += ftc->getPriority();
    }
  }
  return cur_prior;
}
