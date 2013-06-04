#ifndef _COLORER_XMLINPUTSOURCE_H_
#define _COLORER_XMLINPUTSOURCE_H_

#include <xercesc/sax/InputSource.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include<common/Common.h>

class XmlInputSource
{
public:
  static XmlInputSource *newInstance(const XMLCh *path, XmlInputSource *base);
  static XmlInputSource *newInstance(const XMLCh *path, const XMLCh *base);
  virtual XmlInputSource *createRelative(const XMLCh *relPath) const { return nullptr; };
  virtual xercesc::InputSource *getInputSource() {return nullptr;};
  static String *getAbsolutePath(const String*basePath, const String*relPath);
  virtual ~XmlInputSource(){};
protected:
  XmlInputSource(){};
};


#endif