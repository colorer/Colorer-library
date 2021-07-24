#ifndef _COLORER_HRDNODE_H_
#define _COLORER_HRDNODE_H_

#include <colorer/Common.h>
#include <vector>

class HRDNode
{
 public:
  HRDNode() = default;
  ~HRDNode() = default;

  UnicodeString hrd_class;
  UnicodeString hrd_name;
  UnicodeString hrd_description;
  std::vector<UnicodeString> hrd_location;
};

#endif  //_COLORER_HRDNODE_H_
