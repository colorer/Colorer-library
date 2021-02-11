cmake_minimum_required(VERSION 3.15)
project(colorer_lib VERSION ${COLORER_VERSION} LANGUAGES CXX C)

#====================================================
# colorer
#====================================================
set(SRC_COLORER
    colorer/Common.h
    colorer/Exception.h
    colorer/FileType.h
    colorer/HRCParser.h
    colorer/LineSource.h
    colorer/ParserFactory.h
    colorer/Region.h
    colorer/RegionHandler.h
    colorer/Scheme.h
    colorer/TextParser.h
    colorer/version.h
    colorer/common/Exception.cpp
    colorer/common/UStr.cpp
    colorer/common/UStr.h
    colorer/common/UnicodeLogger.h
    colorer/common/UnicodeStringContainer.h
    colorer/cregexp/cregexp.cpp
    colorer/cregexp/cregexp.h
    colorer/editor/BaseEditor.cpp
    colorer/editor/BaseEditor.h
    colorer/editor/EditorListener.h
    colorer/editor/OutlineItem.h
    colorer/editor/Outliner.cpp
    colorer/editor/Outliner.h
    colorer/editor/PairMatch.h
    colorer/handlers/LineRegion.cpp
    colorer/handlers/LineRegion.h
    colorer/handlers/LineRegionsCompactSupport.cpp
    colorer/handlers/LineRegionsCompactSupport.h
    colorer/handlers/LineRegionsSupport.cpp
    colorer/handlers/LineRegionsSupport.h
    colorer/handlers/RegionDefine.h
    colorer/handlers/RegionMapper.cpp
    colorer/handlers/RegionMapper.h
    colorer/handlers/StyledHRDMapper.cpp
    colorer/handlers/StyledHRDMapper.h
    colorer/handlers/StyledRegion.cpp
    colorer/handlers/StyledRegion.h
    colorer/handlers/TextHRDMapper.cpp
    colorer/handlers/TextHRDMapper.h
    colorer/handlers/TextRegion.cpp
    colorer/handlers/TextRegion.h
    colorer/io/FileInputSource.cpp
    colorer/io/FileInputSource.h
    colorer/io/FileWriter.cpp
    colorer/io/FileWriter.h
    colorer/io/InputSource.cpp
    colorer/io/InputSource.h
    colorer/io/SharedInputSource.cpp
    colorer/io/SharedInputSource.h
    colorer/io/StreamWriter.cpp
    colorer/io/StreamWriter.h
    colorer/io/Writer.cpp
    colorer/io/Writer.h
    colorer/parsers/CatalogParser.cpp
    colorer/parsers/CatalogParser.h
    colorer/parsers/FileType.cpp
    colorer/parsers/FileTypeChooser.h
    colorer/parsers/FileTypeImpl.cpp
    colorer/parsers/FileTypeImpl.h
    colorer/parsers/HRCParser.cpp
    colorer/parsers/HRCParserImpl.cpp
    colorer/parsers/HRCParserImpl.h
    colorer/parsers/HRDNode.h
    colorer/parsers/KeywordList.cpp
    colorer/parsers/KeywordList.h
    colorer/parsers/ParserFactory.cpp
    colorer/parsers/ParserFactoryImpl.cpp
    colorer/parsers/ParserFactoryImpl.h
    colorer/parsers/SchemeImpl.h
    colorer/parsers/SchemeNode.cpp
    colorer/parsers/SchemeNode.h
    colorer/parsers/TextParser.cpp
    colorer/parsers/TextParserHelpers.cpp
    colorer/parsers/TextParserHelpers.h
    colorer/parsers/TextParserImpl.cpp
    colorer/parsers/TextParserImpl.h
    colorer/parsers/VirtualEntry.h
    colorer/parsers/XmlTagDefs.h
    colorer/viewer/ParsedLineWriter.h
    colorer/viewer/TextConsoleViewer.cpp
    colorer/viewer/TextConsoleViewer.h
    colorer/viewer/TextLinesStore.cpp
    colorer/viewer/TextLinesStore.h
    colorer/xml/BaseEntityResolver.cpp
    colorer/xml/BaseEntityResolver.h
    colorer/xml/LocalFileXmlInputSource.cpp
    colorer/xml/LocalFileXmlInputSource.h
    colorer/xml/SharedXmlInputSource.cpp
    colorer/xml/SharedXmlInputSource.h
    colorer/xml/XmlInputSource.cpp
    colorer/xml/XmlInputSource.h
    colorer/xml/XmlParserErrorHandler.cpp
    colorer/xml/XmlParserErrorHandler.h
    )

if(COLORER_USE_HTTPINPUTSOURCE AND WIN32)
  set(COLORER_FEATURE_HTTPINPUTSOURCE 1)
  set(SRC_COLORER_HTTP
      colorer/io/HTTPInputSource.cpp
      colorer/io/HTTPInputSource.h
      )
endif()

if(COLORER_USE_JARINPUTSOURCE)
  set(COLORER_FEATURE_JARINPUTSOURCE 1)
  set(SRC_COLORER_JAR
      colorer/io/JARInputSource.cpp
      colorer/io/JARInputSource.h
      colorer/io/MemoryFile.cpp
      colorer/io/MemoryFile.h
      colorer/xml/ZipXmlInputSource.cpp
      colorer/xml/ZipXmlInputSource.h
      )
endif()

# Generate headers
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/colorer/common/Features.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/colorer/common/Features.h
)

#====================================================
# common flags
#====================================================

if(COLORER_STATIC_DEPEND)
  set(COLORER_DEF ${COLORER_DEF} -DXERCES_STATIC_LIBRARY)
endif()
if(COLORER_USE_CONAN)
  set(COLORER_DEF ${COLORER_DEF} -DZLIB_SHORT_PATH)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  # default for gcc
  # CMAKE_CXX_FLAGS=
  # CMAKE_CXX_FLAGS_DEBUG= -g
  # CMAKE_CXX_FLAGS_RELEASE= -O3 -DNDEBUG

  # set only new flags
  set(FLAGS_CXX_DEFAULT -fpermissive -Wall -Wextra -Wpedantic
      -Wsign-promo -Wnon-virtual-dtor)
  set(FLAGS_CXX_RELEASE)
  set(FLAGS_CXX_DEBUG)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")

  # default for msvc
  # CMAKE_C_FLAGS= /DWIN32 /D_WINDOWS
  # CMAKE_C_FLAGS_DEBUG= /MDd /Zi /Ob0 /Od /RTC1
  # CMAKE_C_FLAGS_RELEASE= /MD /O2 /Ob2 /DNDEBUG

  # CMAKE_CXX_FLAGS= /DWIN32 /D_WINDOWS /GR /EHsc
  # CMAKE_CXX_FLAGS_DEBUG= /MDd /Zi /Ob0 /Od /RTC1
  # CMAKE_CXX_FLAGS_RELEASE= /MD /O2 /Ob2 /DNDEBUG

  set(FLAGS_C_DEFAULT)
  set(FLAGS_C_RELEASE /W3)
  set(FLAGS_C_DEBUG /W4)
  set(FLAGS_CXX_DEFAULT)
  set(FLAGS_CXX_RELEASE /W3)
  set(FLAGS_CXX_DEBUG /W4)

  set(COLORER_DEF ${COLORER_DEF} -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
endif()

#====================================================
# build
#====================================================

add_library(colorer_lib STATIC ${SRC_COLORER} ${SRC_COLORER_HTTP} ${SRC_COLORER_JAR})
target_include_directories(colorer_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    )
target_compile_definitions(colorer_lib PUBLIC ${COLORER_DEF})
set_target_properties(colorer_lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )

if(COLORER_USE_CONAN)
  target_include_directories(colorer_lib
      PRIVATE ${CONAN_INCLUDE_DIRS}
      PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
      PUBLIC ../external/spimpl
      )
  target_link_libraries(colorer_lib
      PUBLIC ${CONAN_LIBS}
      )
else()
  target_include_directories(colorer_lib
      PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
      PRIVATE ${SPDLOG_INCLUDE_DIR}
      PRIVATE ${XercesC_INCLUDE_DIRS}
      PRIVATE ${ICU_INCLUDE_DIRS}
      PUBLIC ../external/spimpl
      )
  target_link_libraries(colorer_lib
      PUBLIC ICU::uc XercesC::XercesC spdlog::spdlog
      )

  if(COLORER_FEATURE_JARINPUTSOURCE)
    target_include_directories(colorer_lib
        PRIVATE ${MiniZip_INCLUDE_DIRS}
        )
    target_link_libraries(colorer_lib
        PUBLIC MiniZip::MiniZip
        )
  endif()

endif()

# set options for target
set(MAIN_FLAGS_CXX_DEBUG ${FLAGS_CXX_DEBUG} ${FLAGS_CXX_DEFAULT})
set(MAIN_FLAGS_CXX_RELEASE ${FLAGS_CXX_RELEASE} ${FLAGS_CXX_DEFAULT})
set(MAIN_FLAGS_C_DEBUG ${FLAGS_C_DEBUG} ${FLAGS_C_DEFAULT})
set(MAIN_FLAGS_C_RELEASE ${FLAGS_C_RELEASE} ${FLAGS_C_DEFAULT})

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
  target_compile_options(colorer_lib
      PUBLIC $<$<COMPILE_LANGUAGE:C>:${MAIN_FLAGS_C_DEBUG}>
      PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${MAIN_FLAGS_CXX_DEBUG}>
      )
else()
  target_compile_options(colorer_lib
      PUBLIC $<$<COMPILE_LANGUAGE:C>:${MAIN_FLAGS_C_RELEASE}>
      PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${MAIN_FLAGS_CXX_RELEASE}>
      )
endif()

set_target_properties(colorer_lib PROPERTIES
    OUTPUT_NAME "colorer"
    VERSION ${COLORER_VERSION}
    SOVERSION ${COLORER_VERSION_MAJOR}
    DEBUG_POSTFIX "d"
    )

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