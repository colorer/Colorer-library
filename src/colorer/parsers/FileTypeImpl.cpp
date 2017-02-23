#include <colorer/parsers/FileTypeImpl.h>
#include <colorer/unicode/UnicodeTools.h>

FileTypeImpl::FileTypeImpl(HRCParserImpl* hrcParser): name(nullptr), group(nullptr), description(nullptr)
{
  this->hrcParser = hrcParser;
  protoLoaded = type_loaded = loadDone = load_broken = input_source_loading = false;
  isPackage = false;
  baseScheme = nullptr;
  inputSource = nullptr;
}

FileTypeImpl::~FileTypeImpl(){
  for(auto it : chooserVector){
    delete it;
  }
  for (auto it = chooserVector.begin(); it!=chooserVector.end(); ++it) {
    delete *it;
  }
  chooserVector.clear();

  for(auto it: paramsHash){
    delete it.second;
  }
  paramsHash.clear();

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
  if (tp != paramsHash.end()) return tp->second->description.get();
  return nullptr;
}

const String *FileTypeImpl::getParamValue(const String &name) const{
  auto tp = paramsHash.find(name);
  if (tp != paramsHash.end()){
    if(tp->second->user_value) return tp->second->user_value.get();
    return tp->second->default_value.get();
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
    return tp->second->default_value.get();
  }
  return nullptr;
}

const String* FileTypeImpl::getParamUserValue(const String &name) const{
  auto tp = paramsHash.find(name);
  if (tp !=paramsHash.end()) {
    return tp->second->user_value.get();
  }
  return nullptr;
}

TypeParameter* FileTypeImpl::addParam(const String *name){
  TypeParameter* tp = new TypeParameter;
  tp->name.reset(new SString(name));
  std::pair<SString, TypeParameter*> pp(name, tp);
  paramsHash.emplace(pp);
  return tp;
}

void FileTypeImpl::setParamValue(const String &name, const String *value){
  auto tp = paramsHash.find(name);
  if (tp != paramsHash.end()) {
    tp->second->user_value.reset(new SString(value));
  }
}

void FileTypeImpl::setParamDefaultValue(const String &name, const String *value){
  auto tp = paramsHash.find(name);
  if (tp != paramsHash.end()) {
    tp->second->default_value.reset(new SString(value));
  }
}

void FileTypeImpl::setParamUserValue(const String &name, const String *value){
  setParamValue(name,value);
}

void FileTypeImpl::setParamDescription(const String &name, const String *value){
  auto tp = paramsHash.find(name);
  if (tp != paramsHash.end()) {
    tp->second->description.reset(new SString(value));
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
      cur_prior += ftc->getPriority();
    if (fileContent != nullptr && ftc->isFileContent() && ftc->getRE()->parse(fileContent, &match))
      cur_prior += ftc->getPriority();
  }
  return cur_prior;
}
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
