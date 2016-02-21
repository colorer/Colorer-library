#include <colorer/parsers/SchemeNode.h>

const char* schemeNodeTypeNames[] = { "EMPTY", "RE", "SCHEME", "KEYWORDS", "INHERIT" };

SchemeNode::SchemeNode()
{
  virtualEntryVector.reserve(5);
  type = SNT_EMPTY;
  schemeName = nullptr;
  scheme = nullptr;
  kwList = nullptr;
  worddiv = nullptr;
  start = end = nullptr;
  lowPriority = 0;

  //!!regions cleanup
  region = nullptr;
  memset(regions, 0, sizeof(regions));
  memset(regionsn, 0, sizeof(regionsn));
  memset(regione, 0, sizeof(regione));
  memset(regionen, 0, sizeof(regionen));
};

SchemeNode::~SchemeNode()
{
  if (type == SNT_RE || type == SNT_SCHEME) {
    delete start;
    delete end;
  }
  if (type == SNT_KEYWORDS) {
    delete kwList;
    delete worddiv;
  }
  if (type == SNT_INHERIT) {
    for (auto it : virtualEntryVector) {
      delete it;
    }
    virtualEntryVector.clear();
  }
}