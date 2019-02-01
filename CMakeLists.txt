cmake_minimum_required(VERSION 3.10)
project(colorer CXX)

#====================================================
# global settings
#====================================================
option(COLORER_INTERNAL_BUILD "This CMakeLists.txt called from other" OFF)
option(COLORER_PVS_STUDIO_CHECK "Check project in PVS-Studio" OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type, one of: Release, Debug" FORCE)
endif()
message("Build type: ${CMAKE_BUILD_TYPE}")

if(NOT MSVC_IDE)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
else()
  #target_compile_options cannot set parameters for all configurations
  set(CMAKE_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}" CACHE STRING "" FORCE)
endif()
message("Configurations for IDE: ${CMAKE_CONFIGURATION_TYPES}")       


if(NOT COLORER_INTERNAL_BUILD)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin CACHE STRING "Single Directory for all static libraries" FORCE)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin CACHE STRING "Single Directory for all Executables" FORCE)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin CACHE STRING "Single Directory for all Libraries" FORCE)
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries")

#====================================================
# load modules
#====================================================
# additional modules path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if(COLORER_PVS_STUDIO_CHECK)
  include(PVS-Studio)
endif()

#====================================================
# xercesc
#====================================================
if(WIN32)
  set(XMLCH_TYPE "wchar_t")
else()
  set(XMLCH_TYPE "uint16_t")
endif()
set(xmlch-type ${XMLCH_TYPE} CACHE STRING "XMLCh type wchar_t" FORCE)
set(show-warnings OFF CACHE BOOL "Enable compiler warnings")
set(BUILD_SHARED_RUNTIME OFF CACHE BOOL "Build with shared linked runtine library (MSVC)")
set(XERCESC_BUILD_DOC OFF CACHE BOOL "Build docs")
set(XERCESC_BUILD_TESTS OFF CACHE BOOL "Build test")
set(XERCESC_BUILD_SAMPLES OFF CACHE BOOL "Build samples")

add_subdirectory(./libs/xercesc)

#====================================================
# zlib
#====================================================
set(ZLIB_EXAMPLES OFF CACHE BOOL "Build zlib example programs")
set(ZLIB_MINIZIP ON CACHE BOOL "Build contrib/minizip alongside zlib")
set(ZLIB_BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared (dynamic) libraries")
set(ZLIB_BUILD_SHARED_RUNTIME OFF CACHE BOOL "Build with shared linked runtine library (MSVC)")

add_subdirectory(./libs/zlib)

#====================================================
# colorer library
#====================================================
add_subdirectory(./src)

#====================================================
# colorer tool
#====================================================
option(COLORER_TOOLS_BUILD "build colorer tools" ON)
if(COLORER_TOOLS_BUILD)
  add_subdirectory(./tools/colorer)
endif()