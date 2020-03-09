#ifndef _COLORER_FILETYPEIMPL_H_
#define _COLORER_FILETYPEIMPL_H_

#include <colorer/FileType.h>
#include <colorer/HRCParser.h>
#include <colorer/parsers/FileTypeChooser.h>
#include <colorer/parsers/SchemeImpl.h>
#include <memory>
#include <unordered_map>
#include <vector>

/* structure for storing data of scheme parameter*/
class TypeParameter
{
 public:
  TypeParameter() : name(nullptr), description(nullptr), default_value(nullptr), user_value(nullptr) {};
  ~TypeParameter() = default;

  /* parameter name*/
  uUnicodeString name;
  /* parameter description*/
  uUnicodeString description;
  /* default value*/
  uUnicodeString default_value;
  /* user value*/
  uUnicodeString user_value;
};

/**
 * File Type storage implementation.
 * Contains different attributes of HRC file type.
 * @ingroup colorer_parsers
 */
class FileType::Impl
{
 public:
  Impl();
  ~Impl();

  const UnicodeString* getName() const;
  const UnicodeString* getGroup() const;
  const UnicodeString* getDescription() const;

  void setName(const UnicodeString* name_);
  void setGroup(const UnicodeString* group_);
  void setDescription(const UnicodeString* description_);

  const UnicodeString* getParamValue(const UnicodeString& name_) const;
  const UnicodeString* getParamDefaultValue(const UnicodeString& name_) const;
  const UnicodeString* getParamUserValue(const UnicodeString& name_) const;
  const UnicodeString* getParamDescription(const UnicodeString& name_) const;
  int getParamValueInt(const UnicodeString& name, int def) const;

  void setParamValue(const UnicodeString& name_, const UnicodeString* value);
  void setParamDefaultValue(const UnicodeString& name_, const UnicodeString* value);
  void setParamUserValue(const UnicodeString& name_, const UnicodeString* value);
  void setParamDescription(const UnicodeString& name_, const UnicodeString* value);

  std::vector<UnicodeString> enumParams() const;
  size_t getParamCount() const;
  size_t getParamUserValueCount() const;

  TypeParameter* addParam(const UnicodeString* name_);
  void removeParamValue(const UnicodeString& name_);

  Scheme* getBaseScheme();
  /**
   * Returns total priority, accordingly to all it's
   * choosers (filename and firstline choosers).
   * All <code>fileContent</code> RE's are tested only if priority of previously
   * computed <code>fileName</code> RE's is more, than zero.
   * @param fileName String representation of file name (without path).
   *        If null, method skips filename matching, and starts directly
   *        with fileContent matching.
   * @param fileContent Some part of file's starting content (first line,
   *        for example). If null, skipped.
   * @return Computed total filetype priority.
   */
  double getPriority(const UnicodeString* fileName, const UnicodeString* fileContent) const;

  /// is prototype component loaded
  bool protoLoaded;
  /// is type component loaded
  bool type_loaded;
  /// is type references fully resolved
  bool loadDone;
  /// is initial type load failed
  bool load_broken;
  /// is this IS loading was started
  bool input_source_loading;

  uUnicodeString name;
  uUnicodeString group;
  uUnicodeString description;
  bool isPackage;
  SchemeImpl* baseScheme;

  std::vector<FileTypeChooser*> chooserVector;
  std::unordered_map<UnicodeString, TypeParameter*> paramsHash;
  std::vector<uUnicodeString> importVector;
  uXmlInputSource inputSource;
};

inline const UnicodeString* FileType::Impl::getName() const
{
  return name.get();
}

inline const UnicodeString* FileType::Impl::getGroup() const
{
  return group.get();
}

inline const UnicodeString* FileType::Impl::getDescription() const
{
  return description.get();
}

inline void FileType::Impl::setName(const UnicodeString* name_)
{
  name = std::make_unique<UnicodeString>(*name_);
}

inline void FileType::Impl::setGroup(const UnicodeString* group_)
{
  group = std::make_unique<UnicodeString>(*group_);
}

inline void FileType::Impl::setDescription(const UnicodeString* description_)
{
  description = std::make_unique<UnicodeString>(*description_);
}
#endif
