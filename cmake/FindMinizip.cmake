# - Find minizip
# Find the native MINIZIP includes and library
#
#  MINIZIP_INCLUDE_DIRS - where to find minizip.h, etc.
#  MINIZIP_LIBRARIES   - List of libraries when using minizip.
#  MINIZIP_FOUND       - True if minizip found.
#
#  MiniZip::MiniZip - library, if found


find_path(MiniZip_INCLUDE_DIR NAMES "minizip/zip.h")
mark_as_advanced(MiniZip_INCLUDE_DIR)

find_library(MiniZip_LIBRARY NAMES minizip)
mark_as_advanced(MiniZip_LIBRARY)

include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(MiniZip
    FOUND_VAR MiniZip_FOUND
    REQUIRED_VARS MiniZip_LIBRARY MiniZip_INCLUDE_DIR
    FAIL_MESSAGE "Failed to find MiniZip")

if(MiniZip_FOUND)
  set(MiniZip_LIBRARIES ${MiniZip_LIBRARY})
  set(MiniZip_INCLUDE_DIRS ${MiniZip_INCLUDE_DIR})

  # For header-only libraries
  if(NOT TARGET MiniZip::MiniZip)
    add_library(MiniZip::MiniZip UNKNOWN IMPORTED)
    if(MiniZip_INCLUDE_DIRS)
      set_target_properties(MiniZip::MiniZip PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${MiniZip_INCLUDE_DIRS}")
    endif()
    if(EXISTS "${MiniZip_LIBRARY}")
      set_target_properties(MiniZip::MiniZip PROPERTIES
          IMPORTED_LINK_INTERFACE_LANGUAGES "C"
          IMPORTED_LOCATION "${MiniZip_LIBRARY}")
    endif()
  endif()
endif()


