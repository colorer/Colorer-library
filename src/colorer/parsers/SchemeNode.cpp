#include "colorer/parsers/SchemeNode.h"

SchemeInherit::SchemeInherit() : SchemeNode(SchemeNodeType::SNT_INHERIT)
{
  virtualEntryVector = std::make_unique<VirtualEntryVector>();
}

SchemeInherit::~SchemeInherit()
{
  for (auto it : *virtualEntryVector) {
    delete it;
  }
}
