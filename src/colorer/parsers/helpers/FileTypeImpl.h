#ifndef _COLORER_FILETYPEIMPL_H_
#define _COLORER_FILETYPEIMPL_H_

#include<colorer/parsers/HRCParserImpl.h>
#include<unicode/UnicodeTools.h>

/* structure for storing data of scheme parameter*/
class TypeParameter {
public:
  TypeParameter(): name(nullptr), description(nullptr), default_value(nullptr), user_value(nullptr) {};
  ~TypeParameter() {
    delete name;
    delete description;
    delete default_value;
    delete user_value;
  }

  /* parameter name*/
  String* name;
  /* parameter description*/
  String* description;
  /* default value*/
  String* default_value;
  /* user value*/
  String* user_value;
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
  const String *getName() {
    return name;
  }
  const String *getGroup() {
    return group;
  }
  const String *getDescription() {
    return description;
  }
  Scheme *getBaseScheme() {
    if (!typeLoaded) hrcParser->loadFileType(this);
    return baseScheme;
  }

  const String *enumerateParameters(int idx) {
    TypeParameter* tp = nullptr;
    if (idx==0){
      tp = paramsHash.enumerate();
    }else{
      tp = paramsHash.next();
    }
    if (tp) return tp->name;
    return nullptr;
  }

  const String *getParameterDescription(const String &name) {
    TypeParameter* tp = paramsHash.get(&name);
    if (tp) return tp->description;
    return nullptr;
  }

  const String *getParamValue(const String &name) {
    TypeParameter* tp = paramsHash.get(&name);
    if (tp){
      if(tp->user_value) return tp->user_value;
      return tp->default_value;
    }
    return nullptr;
  }

  int getParamValueInt(const String &name, int def)
  {
    int val = def;
    UnicodeTools::getNumber(getParamValue(name), &val);
    return val;
  }

  const String *getParamDefaultValue(const String &name) {
    TypeParameter* tp = paramsHash.get(&name);
    if (tp) return tp->default_value;
    return nullptr;
  }

  const String *getParamUserValue(const String &name) {
    TypeParameter* tp = paramsHash.get(&name);
    if (tp) return tp->user_value;
    return nullptr;
  }

  TypeParameter* addParam(const String *name){
    TypeParameter* tp = new TypeParameter;
    tp->name = new SString(name);
    paramsHash.put(name, tp);
    return tp;
  }

  void setParamValue(const String &name, const String *value){
    TypeParameter* tp = paramsHash.get(&name);
    if (tp) tp->user_value = new SString(value);
  }

  void setParamDefaultValue(const String &name, const String *value){
    TypeParameter* tp = paramsHash.get(&name);
    if (tp) tp->default_value = new SString(value);
  }

  void setParamDescription(const String &name, const String *value){
    TypeParameter* tp = paramsHash.get(&name);
    if (tp) tp->description = new SString(value);
  }

  void removeParamValue(const String *name){
    paramsHash.remove(name);
  }

  size_t getParamCount(){
    return paramsHash.size();
  }

  size_t getParamUserValueCount(){
    size_t count=0;
    for (TypeParameter* it = paramsHash.enumerate(); it!=nullptr;it=paramsHash.next()){
      if (it && it->user_value) count++;
    }
    return count;
  }
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
  double getPriority(const String *fileName, const String *fileContent) const{
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
protected:
  /// is prototype component loaded
  bool protoLoaded;
  /// is type component loaded
  bool typeLoaded;
  /// is type references fully resolved
  bool loadDone;
  /// is initial type load failed
  bool loadBroken;
  /// is this IS loading was started
  bool inputSourceLoading;

  String *name, *group, *description;
  bool isPackage;
  HRCParserImpl *hrcParser;
  SchemeImpl *baseScheme;

  Vector<FileTypeChooser*> chooserVector;
  Hashtable<TypeParameter*> paramsHash;
  Vector<String*> importVector;
  XmlInputSource *inputSource;

  FileTypeImpl(HRCParserImpl *hrcParser){
    this->hrcParser = hrcParser;
    protoLoaded = typeLoaded = loadDone = loadBroken = inputSourceLoading = false;
    isPackage = false;
    name = group = description = null;
    baseScheme = null;
    inputSource = null;
  }

  ~FileTypeImpl(){
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

};

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
 * Cail Lomecb <cail@nm.ru>.
 * Portions created by the Initial Developer are Copyright (C) 1999-2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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