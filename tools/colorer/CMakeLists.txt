cmake_minimum_required(VERSION 3.15)
project(consoletools VERSION ${COLORER_VERSION} LANGUAGES CXX C)

#====================================================
# colorer_lib
#====================================================
set(SRC_CPP
    ConsoleToolsRunner.cpp
    ConsoleTools.cpp
    ConsoleTools.h
    )

if(WIN32)
  set(SRC_CPP ${SRC_CPP} ConsoleToolsRunner.rc)
endif(WIN32)

#====================================================
# build
#====================================================
#inherit compile options from library

add_executable(consoletools ${SRC_CPP})
target_link_libraries(consoletools PRIVATE colorer_lib)
set_target_properties(consoletools PROPERTIES OUTPUT_NAME "colorer")
set_target_properties(consoletools PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )

if(NOT COLORER_WINx86)
  set(CMAKE_RC_FLAGS -D_WIN64)
endif()

#====================================================
# test
#====================================================

# static analyze
if(COLORER_PVS_STUDIO_CHECK)
  pvs_studio_add_target(TARGET ${PROJECT_NAME}.analyze ALL
      ANALYZE ${PROJECT_NAME}
      MODE GA 64 OP CS
      CFG_TEXT "exclude-path=*/external/*"
      OUTPUT FORMAT errorfile
      LOG ${PROJECT_NAME}.err)
endif()