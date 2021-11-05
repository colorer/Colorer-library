#include <colorer/parsers/SchemeNode.h>

SchemeNode::SchemeNode()
{
  virtualEntryVector.reserve(5);
}

SchemeNode::~SchemeNode()
{
  if (type == SchemeNodeType::SNT_INHERIT) {
    for (auto it : virtualEntryVector) {
      delete it;
    }
    virtualEntryVector.clear();
  }
}