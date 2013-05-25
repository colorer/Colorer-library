cmake_minimum_required (VERSION 2.8.9)
project (consoletools)

#====================================================
# user settings
#====================================================
set(COLORER_FEATURE_HTTPINPUTSOURCE OFF CACHE BOOL "If defined, HTTP InputSource is implemented")

#====================================================
# init variables
#====================================================

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()

if(NOT DEFINED CONF)
  set(CONF "x86")
endif()
        
if(CONF STREQUAL "x86")
  set(PLATFORM "Win32")
else()
  set(PLATFORM "x64")
endif()

#====================================================
# colorer_lib
#====================================================
add_subdirectory(../../src ./build)
include_directories (../../src/)

set(SRC_CPP ConsoleToolsRunner.cpp)

#====================================================
# common flags
#====================================================

if (COLORER_FEATURE_HTTPINPUTSOURCE AND WIN32)
  set(WININETLIB wininet)
endif()

if(PLATFORM STREQUAL "Win32")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWIN32")
else()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWIN64")
endif()


set(FLAGS_RELEASE "-U_DEBUG -DNDEBUG -D_UNICODE -DUNICODE")

set(FLAGS_DEBUG "-D_DEBUG -DDEBUG -D_UNICODE -DUNICODE")

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)

  set(COLORER_WARNING_FLAGS "")
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} /EHsc /Zi")

  set(FLAGS_RELEASE "${FLAGS_RELEASE} /O2 /Oi /Ot /Oy /GL /GF /GS- /Gy /MT")
  set(FLAGS_DEBUG "${FLAGS_DEBUG} /Od /MTd /GS /RTC1")

endif()

if(CMAKE_COMPILER_IS_GNUCXX)
  set(COLORER_WARNING_FLAGS )
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fpermissive")

  set(FLAGS_RELEASE "${FLAGS_RELEASE} -s -O3")
  set(FLAGS_DEBUG "${FLAGS_DEBUG}")

endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COLORER_WARNING_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COLORER_WARNING_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_RELEASE "${FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "${FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_DEBUG "${FLAGS_DEBUG}")

#====================================================
# build
#====================================================

set(LIBRARIES colorer_lib)
set(SRC_FILES ${SRC_CPP})
add_executable (colorer ${SRC_FILES} )
target_link_libraries(colorer ${LIBRARIES} ${WININETLIB})