#include "colorer/parsers/SchemeNode.h"

SchemeNode::~SchemeNode()
{
  if (type == SchemeNodeType::SNT_INHERIT) {
    for (auto it : *virtualEntryVector) {
      delete it;
    }
  }
}