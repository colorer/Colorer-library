#ifndef _COLORER_JARINPUTSOURCE_H_
#define _COLORER_JARINPUTSOURCE_H_

#include<colorer/io/InputSource.h>
#include<colorer/io/SharedInputSource.h>

/** Reads data from JAR(ZIP) archive.
    @ingroup common_io
*/
class JARInputSource : public colorer::InputSource
{
public:
  /** Creates input source from JAR (ZIP) archive.
      @param basePath Path to file in archive in form
             <code>jar:\<anyURI\>!/path/to/file</code>
             @c anyURI can be any valid URI, relative or absolute.
      @param base source, used to resolving relative URIs
  */
  JARInputSource(const UnicodeString *basePath, colorer::InputSource *base);
  ~JARInputSource() override;

  const UnicodeString *getLocation() const override;

  const byte *openStream() override;
  void closeStream() override;
  int length() const override;
protected:
  SharedInputSource *getShared() const { return sharedIS; };
  const UnicodeString *getInJarLocation() const { return inJarLocation; };
  colorer::InputSource *createRelative(const UnicodeString *relPath) override;
  JARInputSource(const UnicodeString *basePath, JARInputSource *base, bool faked);
private:
  UnicodeString *baseLocation;
  UnicodeString *inJarLocation;
  SharedInputSource *sharedIS;
  byte *stream;
  int len;
};

#endif

