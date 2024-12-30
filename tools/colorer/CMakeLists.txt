#====================================================
# consoletools
#====================================================
set(SRC_CPP
    ConsoleToolsRunner.cpp
    ConsoleTools.cpp
    ConsoleTools.h
    SimpleLogger.h
    SimpleLogger.cpp
)

if(WIN32)
  set(SRC_CPP ${SRC_CPP} ConsoleToolsRunner.rc)
endif(WIN32)

#====================================================
# build
#====================================================
#inherit compile options from library

add_executable(consoletools ${SRC_CPP})
add_executable(colorer::consoletools ALIAS consoletools)
target_link_libraries(consoletools PRIVATE colorer::colorer)
set_target_properties(consoletools PROPERTIES OUTPUT_NAME "colorer")
set_target_properties(consoletools PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )

target_compile_definitions(consoletools PUBLIC _CRT_SECURE_NO_WARNINGS)

if(WIN32)
  if("${COLORER_BUILD_ARCH}" STREQUAL "x64")
    set(CMAKE_RC_FLAGS -D_WIN64)
  endif()
endif(WIN32)