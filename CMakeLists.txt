cmake_minimum_required(VERSION 3.15)
# enable CMAKE_MSVC_RUNTIME_LIBRARY
cmake_policy(SET CMP0091 NEW)
# enable <LIB>_ROOT
cmake_policy(SET CMP0074 NEW)

project(colorer CXX)

#====================================================
# global settings
#====================================================
option(COLORER_INTERNAL_BUILD "This CMakeLists.txt called from other" OFF)
option(COLORER_PVS_STUDIO_CHECK "Check project in PVS-Studio" OFF)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type, one of: Release, Debug" FORCE)
endif ()
message("Build type: ${CMAKE_BUILD_TYPE}")

if(NOT MSVC_IDE)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
else()
  #target_compile_options cannot set parameters for all configurations in MSVC
  set(CMAKE_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}" CACHE STRING "" FORCE)
endif()
message("Configurations for IDE: ${CMAKE_CONFIGURATION_TYPES}")

if (NOT COLORER_INTERNAL_BUILD)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
  # set output path for ide.
  if ("${CMAKE_GENERATOR}" MATCHES "Visual Studio")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/vc)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/vc)
  elseif (DEFINED ENV{CLION_IDE} AND MSVC)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/clion_vc_${CMAKE_BUILD_TYPE})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/clion_vc_${CMAKE_BUILD_TYPE})
  elseif (DEFINED ENV{CLION_IDE})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/clion_gcc_${CMAKE_BUILD_TYPE})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/clion_gcc_${CMAKE_BUILD_TYPE})
  endif ()
endif ()

if (MSVC)
  # set global Visual C++ runtime
  if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
  else ()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
  endif ()
endif ()

#====================================================
# load modules
#====================================================
# additional modules path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if (COLORER_PVS_STUDIO_CHECK)
  set(CMAKE_EXPORT_COMPILE_COMMANDS On)
  include(PVS-Studio)
endif ()

#====================================================
# find dependences
#====================================================
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

#====================================================
# colorer library
#====================================================
add_subdirectory(./src)

#====================================================
# colorer tool
#====================================================
option(COLORER_TOOLS_BUILD "build colorer tools" ON)
if (COLORER_TOOLS_BUILD)
  add_subdirectory(./tools/colorer)
endif ()

#====================================================
# unit tests
#====================================================
find_package(GTest)
if (${GTEST_FOUND})
  add_subdirectory(./tests/unit)
endif ()

add_subdirectory(./tests)