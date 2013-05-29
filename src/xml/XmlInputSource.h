#ifndef _COLORER_XMLINPUTSOURCE_H_
#define _COLORER_XMLINPUTSOURCE_H_

#include <xercesc/sax/InputSource.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>

class XmlInputSource
{
public:
  static xercesc::InputSource *newInstance(const XMLCh *path, xercesc::InputSource *base);
protected:
  XmlInputSource(){};
};


#endif