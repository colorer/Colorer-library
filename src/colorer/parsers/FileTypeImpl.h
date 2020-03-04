#ifndef _COLORER_FILETYPEIMPL_H_
#define _COLORER_FILETYPEIMPL_H_

#include <colorer/parsers/FileTypeChooser.h>
#include <colorer/parsers/HRCParserImpl.h>
#include <unordered_map>
#include <vector>

/* structure for storing data of scheme parameter*/
class TypeParameter
{
 public:
  TypeParameter() : name(nullptr), description(nullptr), default_value(nullptr), user_value(nullptr) {};
  ~TypeParameter() {}

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
class FileTypeImpl : public FileType
{
  friend class HRCParserImpl;
  friend class TextParserImpl;

 public:
  const UnicodeString* getName() const;
  const UnicodeString* getGroup() const;
  const UnicodeString* getDescription() const;

  void setName(const UnicodeString* name_);
  void setGroup(const UnicodeString* group_);
  void setDescription(const UnicodeString* description_);

  const UnicodeString* getParamValue(const UnicodeString& name) const;
  const UnicodeString* getParamDefaultValue(const UnicodeString& name) const;
  const UnicodeString* getParamUserValue(const UnicodeString& name) const;
  const UnicodeString* getParamDescription(const UnicodeString& name) const;
  int getParamValueInt(const UnicodeString& name, int def) const;

  void setParamValue(const UnicodeString& name, const UnicodeString* value);
  void setParamDefaultValue(const UnicodeString& name, const UnicodeString* value);
  void setParamUserValue(const UnicodeString& name, const UnicodeString* value);
  void setParamDescription(const UnicodeString& name, const UnicodeString* value);

  std::vector<UnicodeString> enumParams() const;
  size_t getParamCount() const;
  size_t getParamUserValueCount() const;

  TypeParameter* addParam(const UnicodeString* name);
  void removeParamValue(const UnicodeString& name);

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

 protected:
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
  HRCParserImpl* hrcParser;
  SchemeImpl* baseScheme;

  std::vector<FileTypeChooser*> chooserVector;
  std::unordered_map<UnicodeString, TypeParameter*> paramsHash;
  std::vector<uUnicodeString> importVector;
  uXmlInputSource inputSource;

  FileTypeImpl(HRCParserImpl* hrcParser);
  ~FileTypeImpl();
};

inline const UnicodeString* FileTypeImpl::getName() const
{
  return name.get();
}

inline const UnicodeString* FileTypeImpl::getGroup() const
{
  return group.get();
}

inline const UnicodeString* FileTypeImpl::getDescription() const
{
  return description.get();
}

inline void FileTypeImpl::setName(const UnicodeString* name_)
{
  name.reset(new UnicodeString(*name_));
}

inline void FileTypeImpl::setGroup(const UnicodeString* group_)
{
  group.reset(new UnicodeString(*group_));
}

inline void FileTypeImpl::setDescription(const UnicodeString* description_)
{
  description.reset(new UnicodeString(*description_));
}
#endif
