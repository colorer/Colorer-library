#include <colorer/FileType.h>
#include <colorer/parsers/FileTypeImpl.h>

FileType::FileType() : pimpl(new FileType::Impl()) {}

void FileType::setParamValue(const UnicodeString& name, const UnicodeString* value)
{
  pimpl->setParamValue(name, value);
}

const UnicodeString* FileType::getName() const
{
  return pimpl->getName();
}

const UnicodeString* FileType::getGroup() const
{
  return pimpl->getGroup();
}

const UnicodeString* FileType::getDescription() const
{
  return pimpl->getDescription();
}

Scheme* FileType::getBaseScheme()
{
  return pimpl->getBaseScheme();
}

std::vector<UnicodeString> FileType::enumParams() const
{
  return pimpl->enumParams();
}

const UnicodeString* FileType::getParamDescription(const UnicodeString& name) const
{
  return pimpl->getParamDescription(name);
}

const UnicodeString* FileType::getParamValue(const UnicodeString& name) const
{
  return pimpl->getParamValue(name);
}

const UnicodeString* FileType::getParamDefaultValue(const UnicodeString& name) const
{
  return pimpl->getParamDefaultValue(name);
}

int FileType::getParamValueInt(const UnicodeString& name, int def) const
{
  return pimpl->getParamValueInt(name, def);
}

void FileType::addParam(const UnicodeString* name)
{
  pimpl->addParam(name);
}

size_t FileType::getParamCount() const
{
  return pimpl->getParamCount();
}

const UnicodeString* FileType::getParamUserValue(const UnicodeString& name_) const
{
  return pimpl->getParamUserValue(name_);
}

void FileType::setParamDescription(const UnicodeString& name_, const UnicodeString* value)
{
  pimpl->setParamDescription(name_, value);
}

void FileType::setParamDefaultValue(const UnicodeString& name_, const UnicodeString* value)
{
  pimpl->setParamDefaultValue(name_, value);
}
