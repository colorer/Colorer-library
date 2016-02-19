#ifndef _COLORER_ZIPINPUTSOURCE_H_
#define _COLORER_ZIPINPUTSOURCE_H_

#include <unicode/String.h>
#include <xml/XmlInputSource.h>
#include <xercesc/util/BinFileInputStream.hpp>
#include <xml/SharedXmlInputSource.h>

class ZipXmlInputSource : public XmlInputSource
{
public:
  ZipXmlInputSource(const XMLCh* path, const XMLCh* base);
  ZipXmlInputSource(const XMLCh* path, XmlInputSource* base);
  ~ZipXmlInputSource();
  xercesc::BinInputStream* makeStream() const override;
  uXmlInputSource createRelative(const XMLCh* relPath) const override;
  xercesc::InputSource* getInputSource() override;
private:
  void create(const XMLCh* path, const XMLCh* base);
  UString in_jar_location;
  SharedXmlInputSource* jar_input_source;

  ZipXmlInputSource(ZipXmlInputSource const &) = delete;
  ZipXmlInputSource &operator=(ZipXmlInputSource const &) = delete;
  ZipXmlInputSource(ZipXmlInputSource &&) = delete;
  ZipXmlInputSource &operator=(ZipXmlInputSource &&) = delete;
};


class UnZip : public xercesc::BinInputStream
{
public:
  UnZip(const XMLByte* src, XMLSize_t size, const String* path);
  ~UnZip();

  XMLFilePos curPos() const override;
  XMLSize_t readBytes(XMLByte* const toFill, const XMLSize_t maxToRead) override;
  const XMLCh* getContentType() const override;

private:

  XMLSize_t mPos;
  XMLSize_t mBoundary;
  std::unique_ptr<byte[]> stream;
  int len;

  UnZip(UnZip const &) = delete;
  UnZip &operator=(UnZip const &) = delete;
  UnZip(UnZip &&) = delete;
  UnZip &operator=(UnZip &&) = delete;
};

#endif //_COLORER_ZIPINPUTSOURCE_H_
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
