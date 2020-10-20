#ifndef _COLORER_FILETYPE_H_
#define _COLORER_FILETYPE_H_

#include <colorer/Common.h>
#include <colorer/Scheme.h>
#include <vector>

/**
 * HRC FileType (or prototype) instance.
 * @ingroup colorer
 */
class FileType
{
  friend class HRCParser;
  friend class TextParser;

 public:
  /**
   * Public name of file type (HRC 'name' attribute).
   * @return File type Name
   */
  [[nodiscard]] const UnicodeString* getName() const;

  /**
   * Public group name of file type (HRC 'group' attribute).
   * @return File type Group
   */
  [[nodiscard]] const UnicodeString* getGroup() const;

  /** Public description of file type (HRC 'description' attribute).
      @return File type Description
  */
  [[nodiscard]] const UnicodeString* getDescription() const;

  /** Returns the base scheme of this file type.
      Basically, this is the scheme with same public name, as it's type.
      If this FileType object is not yet loaded, it is loaded with this call.
      @return File type base scheme, to be used as root scheme of text parsing.
  */
  Scheme* getBaseScheme();

  /** Enumerates all available parameters, defined in this file type.
      @return Parameter name with index <code>idx</code> or <code>null</code>
      if index is too large.
  */
  [[nodiscard]] std::vector<UnicodeString> enumParams() const;

  [[nodiscard]] const UnicodeString* getParamDescription(const UnicodeString& name) const;
  void setParamDescription(const UnicodeString& name, const UnicodeString* value);

  /** Returns parameter's value of this file type.
      Parameters are stored in prototypes as
      <pre>
      \<parameters>
        \<param name="name" value="value" description="..."/>
      \</parameter>
      </pre>
      Parameters can be used to store application
      specific information about each type of file.
      Also parameters are accessible from the HRC definition
      using <code>if/unless</code> attributes of scheme elements.
      This allows portable customization of HRC loading.
      @param name Parameter's name
      @return Value (changed or default) of this parameter
  */
  [[nodiscard]] const UnicodeString* getParamValue(const UnicodeString& name) const;
  [[nodiscard]] int getParamValueInt(const UnicodeString& name, int def) const;

  /** Returns parameter's default value of this file type.
      Default values are the values, explicitly pointed with
      \c value attribute.
      @param name Parameter's name
      @return Default value of this parameter
  */
  [[nodiscard]] const UnicodeString* getParamDefaultValue(const UnicodeString& name) const;
  [[nodiscard]] const UnicodeString* getParamUserValue(const UnicodeString& name) const;

  /** Changes value of the parameter with specified name.
      Note, that changed parameter values are not stored in HRC
      base - they remains active only during this HRC session.
      Application should use its own mechanism to save these
      values between sessions (if needed).
      @param name Parameter's name
      @param value New value of this parameter.
  */
  void setParamValue(const UnicodeString& name, const UnicodeString* value);
  void setParamDefaultValue(const UnicodeString& name, const UnicodeString* value);

  [[nodiscard]] size_t getParamCount() const;
  void addParam(const UnicodeString* name);

  FileType();
  ~FileType() = default;

 private:
  class Impl;

  std::unique_ptr<Impl> pimpl;
};

#endif
