#ifndef _COLORER_LOCALFILEINPUTSOURCE_H_
#define _COLORER_LOCALFILEINPUTSOURCE_H_

#include <xml/XmlInputSource.h>

class LocalFileXmlInputSource : public xercesc::LocalFileInputSource, public XmlInputSource 
{
public:
  LocalFileXmlInputSource (const XMLCh *path, const XMLCh *base);
  ~LocalFileXmlInputSource();
  xercesc::BinInputStream* makeStream() const;
  XmlInputSource *createRelative(const XMLCh *relPath) const;
  xercesc::InputSource *getInputSource();
private:
  XMLCh *ExpandEnvironment(const XMLCh *path);
};


#endif
