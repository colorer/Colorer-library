#ifndef _COLORER_FILETYPECHOOSER_H_
#define _COLORER_FILETYPECHOOSER_H_

#include <colorer/cregexp/cregexp.h>

/** Stores regular expressions of filename and firstline
    elements and helps to detect file type.
    @ingroup colorer_parsers
*/
class FileTypeChooser
{
public:
  enum class ChooserType { CT_FILENAME, CT_FIRSTLINE };

  /** Creates choose entry.
      @param type If 0 - filename RE, if 1 - firstline RE
      @param prior Priority of this rule
      @param re Associated regular expression
  */
  FileTypeChooser(ChooserType type, double prior, CRegExp* re);
  /** Default destructor */
  ~FileTypeChooser() = default;
  /** Returns type of chooser */
  [[nodiscard]] bool isFileName() const;
  /** Returns type of chooser */
  [[nodiscard]] bool isFileContent() const;
  /** Returns chooser priority */
  [[nodiscard]] double getPriority() const;
  /** Returns associated regular expression */
  [[nodiscard]] CRegExp* getRE() const;
private:
  std::unique_ptr<CRegExp> reg_matcher;
  ChooserType type;
  double priority;
};

inline FileTypeChooser::FileTypeChooser(ChooserType type_, double prior, CRegExp* re):
  reg_matcher(re), type(type_), priority(prior)
{
}

inline bool FileTypeChooser::isFileName() const
{
  return type == ChooserType::CT_FILENAME;
}

inline bool FileTypeChooser::isFileContent() const
{
  return type == ChooserType::CT_FIRSTLINE;
}

inline double FileTypeChooser::getPriority() const
{
  return priority;
}

inline CRegExp* FileTypeChooser::getRE() const
{
  return reg_matcher.get();
}

#endif //_COLORER_FILETYPECHOOSER_H_


