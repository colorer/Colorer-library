#include <colorer/xml/LocalFileXmlInputSource.h>
#include <xercesc/util/BinFileInputStream.hpp>
#include <xercesc/util/XMLString.hpp>
#include "XStr.h"

LocalFileXmlInputSource::LocalFileXmlInputSource(const XMLCh* path, const XMLCh* base)
{
  input_source.reset(new xercesc::LocalFileInputSource(base, path));
  if (xercesc::XMLString::findAny(path, kPercent) != nullptr) {
    XMLCh* e_path = ExpandEnvironment(path);
    input_source->setSystemId(e_path);
    delete e_path;
  }
}

LocalFileXmlInputSource::~LocalFileXmlInputSource()
{
}

xercesc::BinInputStream* LocalFileXmlInputSource::makeStream() const
{
  std::unique_ptr<xercesc::BinFileInputStream> stream(new xercesc::BinFileInputStream(input_source->getSystemId()));
  if (!stream->getIsOpen()) {
    throw InputSourceException(SString("Can't open file '") + DString(input_source->getSystemId()) + "'");
  }
  return stream.release();
}

uXmlInputSource LocalFileXmlInputSource::createRelative(const XMLCh* relPath) const
{
  return std::make_unique<LocalFileXmlInputSource>(relPath, input_source->getSystemId());
}

xercesc::InputSource* LocalFileXmlInputSource::getInputSource()
{
  return input_source.get();
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
