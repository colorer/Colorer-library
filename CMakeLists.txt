cmake_minimum_required(VERSION 3.15)

#====================================================
# Enable policy
#====================================================
# enable CMAKE_MSVC_RUNTIME_LIBRARY
cmake_policy(SET CMP0091 NEW)
# enable <LIB>_ROOT
cmake_policy(SET CMP0074 NEW)

#====================================================
project(colorer CXX)

#====================================================
# Set default build type
#====================================================
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type, one of: Release, Debug" FORCE)
endif()
message("Build type: ${CMAKE_BUILD_TYPE}")

#====================================================
# global library settings
#====================================================
option(COLORER_INTERNAL_BUILD "This CMakeLists.txt called from other" OFF)
option(COLORER_PVS_STUDIO_CHECK "Check project in PVS-Studio" OFF)
option(COLORER_USE_CONAN "Use Conan for dependencies" OFF)
option(COLORER_WINx86 "Set on if building for Windows x86" OFF)
option(COLORER_BUILD_TEST "Set on to build test" OFF)

#====================================================
# global compilation settings
#====================================================
if(MSVC)
  # set global Visual C++ runtime
  if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
  else()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
  endif()
endif()

#====================================================
# load modules
#====================================================
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(cmake/ide.cmake)
include(cmake/utils.cmake)

colorer_extract_version()
message(STATUS "Build Colorer: ${COLORER_VERSION}")

#====================================================
# find dependences
#====================================================
if(COLORER_USE_CONAN)
  include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
  conan_basic_setup()
else()
  find_package(ICU COMPONENTS uc REQUIRED)
  find_package(XercesC REQUIRED)
  find_package(spdlog REQUIRED)
  find_package(fmt REQUIRED)
  find_package(minizip REQUIRED)
  find_package(GTest)
endif()

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

#====================================================
# PVS-Studio
#====================================================
if(COLORER_PVS_STUDIO_CHECK)
  set(CMAKE_EXPORT_COMPILE_COMMANDS On)
  include(cmake/PVS-Studio.cmake)
endif()

#====================================================
# Tests
#====================================================
if(COLORER_BUILD_TEST)
  if(${GTEST_FOUND})
    add_subdirectory(./tests/unit)
  endif()

  add_subdirectory(./tests)
endif()