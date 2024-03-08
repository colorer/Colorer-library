#include "colorer/parsers/SchemeNode.h"

SchemeNode::SchemeNode(SchemeNodeType _type) : type(_type)
{
  if (type == SchemeNodeType::SNT_INHERIT) {
    virtualEntryVector = std::make_unique<VirtualEntryVector>();
  }
}

SchemeNode::~SchemeNode()
{
  if (type == SchemeNodeType::SNT_INHERIT) {
    for (auto it : *virtualEntryVector) {
      delete it;
    }
  }
}
