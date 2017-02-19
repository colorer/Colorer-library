cmake_minimum_required(VERSION 2.8.11)
project(colorer CXX)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
message("Configurations for IDE: ${CMAKE_CONFIGURATION_TYPES}")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type, one of: Release, Debug" FORCE)
endif()
message("Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin CACHE STRING "Single Directory for all static libraries" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin CACHE STRING "Single Directory for all Executables" FORCE)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin CACHE STRING "Single Directory for all Libraries" FORCE)

set(ADD_FATAL_EXAMPLE OFF CACHE BOOL "Build g3log without examples")
set(USE_DYNAMIC_LOGGING_LEVELS ON CACHE BOOL "Build g3log with dynamic logging levels")
add_subdirectory(./libs/g3log)

add_subdirectory(./libs/xercesc)

add_subdirectory(./src)
add_subdirectory(./tools/colorer)