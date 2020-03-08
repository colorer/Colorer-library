#ifndef _COLORER_VIRTUALENTRY_H_
#define _COLORER_VIRTUALENTRY_H_

class SchemeImpl;

/** One entry of 'inherit' element virtualization content.
    @ingroup colorer_parsers
*/
class VirtualEntry
{
public:
  SchemeImpl* virtScheme;
  SchemeImpl* substScheme;
  uUnicodeString virtSchemeName;
  uUnicodeString substSchemeName;

  VirtualEntry(const UnicodeString* scheme, const UnicodeString* subst)
  {
    virtScheme = substScheme = nullptr;
    virtSchemeName.reset(new UnicodeString(*scheme));
    substSchemeName.reset(new UnicodeString(*subst));
  }

  ~VirtualEntry() = default;


};

#endif // _COLORER_VIRTUALENTRY_H_

