
#include<colorer/io/JARInputSource.h>
#include<colorer/io/MemoryFile.h>
#include<contrib/minizip/unzip.h>

JARInputSource::JARInputSource(const String *basePath, InputSource *base){
  if (basePath == nullptr)
    throw InputSourceException(SString("Can't create jar source"));
  // absolute jar uri
  int ex_idx = basePath->lastIndexOf('!');
  if (ex_idx == -1) throw InputSourceException(SString("Bad jar uri format: ") + basePath);

  inJarLocation = new SString(basePath, ex_idx+1, -1);
  
  DString bpath = DString(basePath, 4, ex_idx-4);
  sharedIS = SharedInputSource::getInputSource(&bpath, base);

  SString str("jar:");
  str.append(sharedIS->getLocation());
  str.append(DString("!"));
  str.append(inJarLocation);
  baseLocation = new SString(&str);

  stream = nullptr;
  len = 0;
}

JARInputSource::~JARInputSource(){
  sharedIS->delref();
  delete baseLocation;
  delete inJarLocation;
  delete stream;
}

JARInputSource::JARInputSource(const String *basePath, JARInputSource *base, bool faked){
  // relative jar uri
  JARInputSource *parent = base;
  if (parent == nullptr) throw InputSourceException(SString("Bad jar uri format: ") + basePath);
  sharedIS = parent->getShared();
  sharedIS->addref();

  inJarLocation = getAbsolutePath(parent->getInJarLocation(), basePath);

  SString str("jar:");
  str.append(sharedIS->getLocation());
  str.append(DString("!"));
  str.append(inJarLocation);
  baseLocation = new SString(&str);
  stream = nullptr;
  len = 0;
}

colorer::InputSource *JARInputSource::createRelative(const String *relPath){
  return new JARInputSource(relPath, this, true);
}

const String *JARInputSource::getLocation() const{
  return baseLocation;
}

const byte *JARInputSource::openStream()
{
  if (stream != nullptr)
    throw InputSourceException(SString("openStream(): source stream already opened: '")+baseLocation+"'");

  MemoryFile *mf = new MemoryFile;
  mf->stream = sharedIS->getStream();
  mf->length = sharedIS->length();
  zlib_filefunc_def zlib_ff;
  fill_mem_filefunc(&zlib_ff, mf);

  unzFile fid = unzOpen2(nullptr, &zlib_ff);

  if (fid == 0) {
	  delete mf;
	  unzClose(fid);
	  throw InputSourceException(SString("Can't locate file in JAR content: '")+inJarLocation+"'");
  }
  int ret = unzLocateFile(fid, inJarLocation->getChars(), 0);
  if (ret != UNZ_OK)  {
	  delete mf;
	  unzClose(fid);
	  throw InputSourceException(SString("Can't locate file in JAR content: '")+inJarLocation+"'");
  }
  unz_file_info file_info;
  ret = unzGetCurrentFileInfo(fid, &file_info, nullptr, 0, nullptr, 0, nullptr, 0);
  if (ret != UNZ_OK)  {
	  delete mf;
	  unzClose(fid);
	  throw InputSourceException(SString("Can't retrieve current file in JAR content: '")+inJarLocation+"'");
  }

  len = file_info.uncompressed_size;
  stream = new byte[len];
  ret = unzOpenCurrentFile(fid);
  if (ret != UNZ_OK)  {
	  delete mf;
	  unzClose(fid);
	  throw InputSourceException(SString("Can't open current file in JAR content: '")+inJarLocation+"'");
  }
  ret = unzReadCurrentFile(fid, stream, len);
  if (ret <= 0) {
	  delete mf;
	  unzClose(fid);
	  throw InputSourceException(SString("Can't read current file in JAR content: '")+inJarLocation+"' ("+SString(ret)+")");
  }
  ret = unzCloseCurrentFile(fid);
  if (ret == UNZ_CRCERROR) {
	  delete mf;
	  unzClose(fid);
	  throw InputSourceException(SString("Bad JAR file CRC"));
  }
  ret = unzClose(fid);
  delete mf;
  return stream;
}

void JARInputSource::closeStream(){
  if (stream == nullptr)
    throw InputSourceException(SString("closeStream(): source stream is not yet opened"));
  delete stream;
  stream = nullptr;
}

int JARInputSource::length() const{
  if (stream == nullptr)
    throw InputSourceException(DString("length(): stream is not yet opened"));
  return len;
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
