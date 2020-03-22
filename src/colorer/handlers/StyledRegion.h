#ifndef _COLORER_STYLEDREGION_H_
#define _COLORER_STYLEDREGION_H_

#include <colorer/handlers/RegionDefine.h>

/**
 * Contains information about region mapping into real colors.
 * These mappings are stored in HRD files and processed
 * by StyledHRDMapper class.
 * @ingroup colorer_handlers
 */
class StyledRegion : public RegionDefine
{
 public:
  static const int RD_BOLD;
  static const int RD_ITALIC;
  static const int RD_UNDERLINE;
  static const int RD_STRIKEOUT;

  /** Is foreground value assigned? */
  bool bfore;
  /** Is background value assigned? */
  bool bback;
  /** Foreground color of region */
  unsigned int fore;
  /** Background color of region */
  unsigned int back;
  /** Bit mask of region's style (bold, italic, underline) */
  unsigned int style;

  /** Common constructor */
  StyledRegion(bool _bfore, bool _bback, unsigned int _fore, unsigned int _back, unsigned int _style);

  /** Empty constructor */
  StyledRegion();

  /** Copy constructor.
      Clones all values including region reference. */
  StyledRegion(const StyledRegion& rd);

  StyledRegion& operator=(const StyledRegion& rd);

  ~StyledRegion() override = default;

  /** Static method, used to cast RegionDefine class into
      StyledRegion class.
      @throw Exception If casing is not available.
  */
  static const StyledRegion* cast(const RegionDefine* rd);

  /** Completes region define with it's parent values.
      The values only replaced, are these, which are empty
      in this region define. Style is replaced using OR operation.
  */
  void assignParent(const RegionDefine* _parent) override;

  void setValues(const RegionDefine* _rd) override;

  [[nodiscard]] RegionDefine* clone() const override;
};

#endif
