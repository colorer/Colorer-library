#ifndef _COLORER_SHAREDXMLINPUTSOURCE_H_
#define _COLORER_SHAREDXMLINPUTSOURCE_H_

#include <colorer/Common.h>
#include <xercesc/sax/InputSource.hpp>
#include <colorer/xml/XmlInputSource.h>

class SharedXmlInputSource
{
public:

  static SharedXmlInputSource* getSharedInputSource(const XMLCh* path, const XMLCh* base);
  xercesc::InputSource* getInputSource() const;

  /** Increments reference counter */
  int addref();
  /** Decrements reference counter */
  int delref();

  XMLSize_t getSize() const;
  XMLByte* getSrc() const;
private:
  SharedXmlInputSource(uXmlInputSource &source);
  ~SharedXmlInputSource();

  static std::unordered_map<SString, SharedXmlInputSource*>* isHash;

  uXmlInputSource input_source;
  int ref_count;
  std::unique_ptr<XMLByte[]> mSrc;
  XMLSize_t mSize;

  SharedXmlInputSource(SharedXmlInputSource const &) = delete;
  SharedXmlInputSource &operator=(SharedXmlInputSource const &) = delete;
  SharedXmlInputSource(SharedXmlInputSource &&) = delete;
  SharedXmlInputSource &operator=(SharedXmlInputSource &&) = delete;
};

inline XMLSize_t SharedXmlInputSource::getSize() const
{
  return mSize;
}

inline XMLByte* SharedXmlInputSource::getSrc() const
{
  return mSrc.get();
}

#endif //_COLORER_SHAREDXMLINPUTSOURCE_H_

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
