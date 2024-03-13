#include "colorer/parsers/SchemeNode.h"

SchemeInherit::~SchemeInherit()
{
  for (auto it : virtualEntryVector) {
    delete it;
  }
}
