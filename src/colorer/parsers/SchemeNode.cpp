#include <colorer/parsers/SchemeNode.h>

const char* schemeNodeTypeNames[] = {"EMPTY", "RE", "SCHEME", "KEYWORDS", "INHERIT"};

SchemeNode::SchemeNode()
{
  virtualEntryVector.reserve(5);
}

SchemeNode::~SchemeNode()
{
  if (type == SNT_INHERIT) {
    for (auto it : virtualEntryVector) {
      delete it;
    }
    virtualEntryVector.clear();
  }
}