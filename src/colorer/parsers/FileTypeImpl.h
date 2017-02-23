#ifndef _COLORER_FILETYPEIMPL_H_
#define _COLORER_FILETYPEIMPL_H_

#include <vector>
#include <unordered_map>
#include <colorer/parsers/HRCParserImpl.h>
#include <colorer/parsers/FileTypeChooser.h>

/* structure for storing data of scheme parameter*/
class TypeParameter {
public:
  TypeParameter(): name(nullptr), description(nullptr), default_value(nullptr), user_value(nullptr) {};
  ~TypeParameter() {}

  /* parameter name*/
  UString name;
  /* parameter description*/
  UString description;
  /* default value*/
  UString default_value;
  /* user value*/
  UString user_value;
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
  const String *getName() const;
  const String *getGroup() const;
  const String *getDescription() const;

  void setName(const String *name_);
  void setGroup(const String *group_);
  void setDescription(const String *description_);

  const String *getParamValue(const String &name) const;
  const String *getParamDefaultValue(const String &name) const;
  const String *getParamUserValue(const String &name) const;
  const String *getParamDescription(const String &name) const;
  int getParamValueInt(const String &name, int def) const;

  void setParamValue(const String &name, const String *value);
  void setParamDefaultValue(const String &name, const String *value);
  void setParamUserValue(const String &name, const String *value);
  void setParamDescription(const String &name, const String *value);

  std::vector<SString> enumParams() const;
  size_t getParamCount() const;
  size_t getParamUserValueCount() const;

  TypeParameter* addParam(const String *name);
  void removeParamValue(const String &name);

  Scheme *getBaseScheme();
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
  double getPriority(const String *fileName, const String *fileContent) const;
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

  UString name;
  UString group;
  UString description;
  bool isPackage;
  HRCParserImpl *hrcParser;
  SchemeImpl *baseScheme;

  std::vector<FileTypeChooser*> chooserVector;
  std::unordered_map<SString, TypeParameter*> paramsHash;
  std::vector<UString> importVector;
  uXmlInputSource inputSource;

  FileTypeImpl(HRCParserImpl *hrcParser);
  ~FileTypeImpl();
};

inline const String* FileTypeImpl::getName() const{
  return name.get();
}

inline const String* FileTypeImpl::getGroup() const{
  return group.get();
}

inline const String* FileTypeImpl::getDescription() const{
  return description.get();
}

inline void FileTypeImpl::setName(const String *name_) {
  name.reset(new SString(name_));
}

inline void FileTypeImpl::setGroup(const String *group_) {
  group.reset(new SString(group_));
}

inline void FileTypeImpl::setDescription(const String *description_) {
  description.reset(new SString(description_));
}
#endif
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Colorer Library.
 *
 * The Initial Developer of the Original Code is
 * Cail Lomecb <irusskih at gmail dot com>.
 * Portions created by the Initial Developer are Copyright (C) 1999-2009
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s): see file CONTRIBUTORS
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
