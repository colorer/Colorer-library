#ifndef _COLORER_REGION_H_
#define _COLORER_REGION_H_

#include <colorer/Common.h>

/**
  HRC Region implementation.
  Contains information about HRC Region and it attributes:
  <ul>
    <li>name
    <li>description
    <li>parent
  </ul>
  @ingroup colorer
*/
class Region
{
 public:
  /** Full Qualified region name (<code>def:Text</code> for example) */
  [[nodiscard]] virtual const UnicodeString* getName() const
  {
    return name;
  }
  /** Region description */
  [[nodiscard]] virtual const UnicodeString* getDescription() const
  {
    return description;
  }
  /** Direct region ancestor (parent) */
  [[nodiscard]] virtual const Region* getParent() const
  {
    return parent;
  }
  /** Quick access region id (incrementable) */
  [[nodiscard]] virtual size_t getID() const
  {
    return id;
  }
  /** Checks if region has the specified parent in all of it's ancestors.
      This method is useful to check if region has specified parent,
      and use this information, as region type specification.
      For example, <code>def:Comment</code> has <code>def:Syntax</code> parent,
      so, some syntax checking can be made with it's content.
  */
  bool hasParent(const Region* region) const
  {
    const Region* elem = this;
    while (elem != nullptr) {
      if (region == elem) {
        return true;
      }
      elem = elem->getParent();
    }
    return false;
  }
  /**
    Basic constructor.
    Used only by HrcLibrary.
  */
  Region(const UnicodeString* _name, const UnicodeString* _description, const Region* _parent, size_t _id)
  {
    name = new UnicodeString(*_name);
    description = nullptr;
    if (_description != nullptr) {
      description = new UnicodeString(*_description);
    }
    parent = _parent;
    id = _id;
  }

  virtual ~Region()
  {
    delete name;
    delete description;
  }

 protected:
  /** Internal members */
  UnicodeString *name, *description;
  const Region* parent;
  size_t id;
};

#endif
