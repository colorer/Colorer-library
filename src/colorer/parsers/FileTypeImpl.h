#ifndef _COLORER_FILETYPEIMPL_H_
#define _COLORER_FILETYPEIMPL_H_

#include <colorer/FileType.h>
#include <colorer/HrcLibrary.h>
#include <colorer/parsers/FileTypeChooser.h>
#include <colorer/parsers/SchemeImpl.h>
#include <memory>
#include <unordered_map>
#include <vector>

/* structure for storing data of scheme parameter*/
class TypeParameter
{
 public:
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
  [[nodiscard]] const UnicodeString* getName() const;
  [[nodiscard]] const UnicodeString* getGroup() const;
  [[nodiscard]] const UnicodeString* getDescription() const;

  void setName(const UnicodeString* param_name);
  void setGroup(const UnicodeString* group_name);
  void setDescription(const UnicodeString* description);

  [[nodiscard]] const UnicodeString* getParamValue(const UnicodeString& param_name) const;
  [[nodiscard]] const UnicodeString* getParamDefaultValue(const UnicodeString& param_name) const;
  [[nodiscard]] const UnicodeString* getParamUserValue(const UnicodeString& param_name) const;
  [[nodiscard]] const UnicodeString* getParamDescription(const UnicodeString& param_name) const;
  [[nodiscard]] int getParamValueInt(const UnicodeString& param_name, int def) const;

  void setParamValue(const UnicodeString& param_name, const UnicodeString* value);
  void setParamDefaultValue(const UnicodeString& param_name, const UnicodeString* value);
  void setParamUserValue(const UnicodeString& param_name, const UnicodeString* value);
  void setParamDescription(const UnicodeString& param_name, const UnicodeString* value);

  [[nodiscard]] std::vector<UnicodeString> enumParams() const;
  [[nodiscard]] size_t getParamCount() const;
  [[nodiscard]] size_t getParamUserValueCount() const;

  TypeParameter* addParam(const UnicodeString* param_name);
  void removeParamValue(const UnicodeString& param_name);

  [[nodiscard]] Scheme* getBaseScheme() const;
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
  bool protoLoaded = false;
  /// is type component loaded
  bool type_loaded = false;
  /// is type references fully resolved
  bool loadDone = false;
  /// is initial type load failed
  bool load_broken = false;
  /// is this IS loading was started
  bool input_source_loading = false;

  uUnicodeString name;
  uUnicodeString group;
  uUnicodeString description;
  bool isPackage = false;
  SchemeImpl* baseScheme = nullptr;

  std::vector<std::unique_ptr<FileTypeChooser>> chooserVector;
  std::unordered_map<UnicodeString, std::unique_ptr<TypeParameter>> paramsHash;
  std::vector<uUnicodeString> importVector;
  uXmlInputSource inputSource;
};
#endif
