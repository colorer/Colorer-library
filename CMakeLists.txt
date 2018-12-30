cmake_minimum_required(VERSION 2.8.11)
project(colorer CXX)

#====================================================
# global settings
#====================================================
option(COLORER_INTERNAL_BUILD "This CMakeLists.txt called from other" OFF)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
message("Configurations for IDE: ${CMAKE_CONFIGURATION_TYPES}")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type, one of: Release, Debug" FORCE)
endif()
message("Build type: ${CMAKE_BUILD_TYPE}")

if(NOT COLORER_INTERNAL_BUILD)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin CACHE STRING "Single Directory for all static libraries" FORCE)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin CACHE STRING "Single Directory for all Executables" FORCE)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin CACHE STRING "Single Directory for all Libraries" FORCE)
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries")

#====================================================
# g3log
#====================================================
set(ADD_FATAL_EXAMPLE OFF CACHE BOOL "Build g3log without examples")
set(USE_DYNAMIC_LOGGING_LEVELS ON CACHE BOOL "Build g3log with dynamic logging levels")
set(G3_SHARED_LIB OFF CACHE BOOL "Build shared library")
set(G3_SHARED_RUNTIME OFF CACHE BOOL "Build with shared linked runtine library (MSVC)")

add_subdirectory(./libs/g3log)

#====================================================
# xercesc
#====================================================
if(WIN32)
  set(XMLCH_TYPE "wchar_t")
else()
  set(XMLCH_TYPE "uint16_t")
endif()
set(xmlch-type ${XMLCH_TYPE} CACHE STRING "XMLCh type wchar_t" FORCE)
set(BUILD_SHARED_RUNTIME OFF CACHE BOOL "Build with shared linked runtine library (MSVC)")
set(XERCESC_BUILD_DOC OFF CACHE BOOL "Build docs")
set(XERCESC_BUILD_TESTS OFF CACHE BOOL "Build test")
set(XERCESC_BUILD_SAMPLES OFF CACHE BOOL "Build samples")

add_subdirectory(./libs/xercesc)

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