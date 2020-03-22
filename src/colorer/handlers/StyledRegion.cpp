#include <colorer/Exception.h>
#include <colorer/handlers/StyledRegion.h>

StyledRegion::StyledRegion(bool _bfore, bool _bback, unsigned int _fore, unsigned int _back, unsigned int _style)
{
  type = RegionDefine::STYLED_REGION;
  bfore = _bfore;
  bback = _bback;
  fore = _fore;
  back = _back;
  style = _style;
}

StyledRegion::StyledRegion()
{
  type = RegionDefine::STYLED_REGION;
  bfore = bback = false;
  fore = back = 0;
  style = 0;
}

StyledRegion::StyledRegion(const StyledRegion& rd)
{
  operator=(rd);
}

StyledRegion& StyledRegion::operator=(const StyledRegion& rd)
{
  if (this == &rd)
    return *this;
  setValues(&rd);
  return *this;
}

const StyledRegion* StyledRegion::cast(const RegionDefine* rd)
{
  if (rd == nullptr)
    return nullptr;
  if (rd->type != RegionDefine::STYLED_REGION)
    throw Exception("Bad type cast exception into StyledRegion");
  const auto* sr = (const StyledRegion*) (rd);
  return sr;
}

void StyledRegion::assignParent(const RegionDefine* _parent)
{
  const StyledRegion* parent = StyledRegion::cast(_parent);
  if (parent == nullptr)
    return;
  if (!bfore) {
    fore = parent->fore;
    bfore = parent->bfore;
  }
  if (!bback) {
    back = parent->back;
    bback = parent->bback;
  }
  style = style | parent->style;
}

void StyledRegion::setValues(const RegionDefine* _rd)
{
  const StyledRegion* rd = StyledRegion::cast(_rd);
  if (rd) {
    fore = rd->fore;
    bfore = rd->bfore;
    back = rd->back;
    bback = rd->bback;
    style = rd->style;
    type = rd->type;
  }
}

RegionDefine* StyledRegion::clone() const
{
  RegionDefine* rd = new StyledRegion(*this);
  return rd;
}