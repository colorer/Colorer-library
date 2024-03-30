#====================================================
# colorer source
#====================================================
set(SRC_COLORER
    colorer/Common.h
    colorer/Exception.h
    colorer/FileType.h
    colorer/HrcLibrary.h
    colorer/LineSource.h
    colorer/ParserFactory.h
    colorer/Region.h
    colorer/RegionHandler.h
    colorer/Scheme.h
    colorer/TextParser.h
    colorer/base/BaseNames.h
    colorer/base/XmlTagDefs.h
    colorer/common/Character.cpp
    colorer/common/Character.h
    colorer/common/Encodings.cpp
    colorer/common/Encodings.h
    colorer/common/Exception.cpp
    colorer/common/UStr.cpp
    colorer/common/UStr.h
    colorer/common/UnicodeLogger.h
    colorer/common/UnicodeTools.cpp
    colorer/common/UnicodeTools.h
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
    colorer/io/StreamWriter.cpp
    colorer/io/StreamWriter.h
    colorer/io/Writer.cpp
    colorer/io/Writer.h
    colorer/parsers/CatalogParser.cpp
    colorer/parsers/CatalogParser.h
    colorer/parsers/FileType.cpp
    colorer/parsers/FileTypeChooser.cpp
    colorer/parsers/FileTypeChooser.h
    colorer/parsers/FileTypeImpl.cpp
    colorer/parsers/FileTypeImpl.h
    colorer/parsers/HrcLibrary.cpp
    colorer/parsers/HrcLibraryImpl.cpp
    colorer/parsers/HrcLibraryImpl.h
    colorer/parsers/HrdNode.h
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
    colorer/utils/Environment.cpp
    colorer/utils/Environment.h
    colorer/version.h
    colorer/viewer/ParsedLineWriter.cpp
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

if(COLORER_USE_JARINPUTSOURCE)
  set(COLORER_FEATURE_JARINPUTSOURCE 1)
  set(SRC_COLORER_JAR
      colorer/xml/MemoryFile.cpp
      colorer/xml/MemoryFile.h
      colorer/xml/ZipXmlInputSource.cpp
      colorer/xml/ZipXmlInputSource.h
      )
endif()

#====================================================
# Generate headers
#====================================================
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/colorer/common/Features.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/colorer/common/Features.h
)

#====================================================
# common flags
#====================================================
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  # default for gcc
  # CMAKE_CXX_FLAGS=
  # CMAKE_CXX_FLAGS_DEBUG= -g
  # CMAKE_CXX_FLAGS_RELEASE= -O3 -DNDEBUG

  # set only new flags
  set(FLAGS_CXX_DEFAULT -fpermissive -Wall -Wextra -Wpedantic -Wsign-promo -Wnon-virtual-dtor)
  set(FLAGS_CXX_RELEASE -Werror)
  set(FLAGS_CXX_DEBUG)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")

  # default for msvc
  # CMAKE_CXX_FLAGS= /DWIN32 /D_WINDOWS /GR /EHsc
  # CMAKE_CXX_FLAGS_DEBUG= /MDd /Zi /Ob0 /Od /RTC1
  # CMAKE_CXX_FLAGS_RELEASE= /MD /O2 /Ob2 /DNDEBUG

  set(FLAGS_CXX_DEFAULT)
  set(FLAGS_CXX_RELEASE /W3 /WX)
  set(FLAGS_CXX_DEBUG /W4)
endif()

set(MAIN_FLAGS_CXX_DEBUG ${FLAGS_CXX_DEBUG} ${FLAGS_CXX_DEFAULT})
set(MAIN_FLAGS_CXX_RELEASE ${FLAGS_CXX_RELEASE} ${FLAGS_CXX_DEFAULT})

#====================================================
# build
#====================================================
add_library(colorer_lib STATIC ${SRC_COLORER} ${SRC_COLORER_HTTP} ${SRC_COLORER_JAR})
add_library(colorer::colorer ALIAS colorer_lib)
target_include_directories(colorer_lib PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

set_target_properties(colorer_lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )

target_link_libraries(colorer_lib
    PUBLIC ICU::uc ICU::data XercesC::XercesC spdlog::spdlog
    )

if(COLORER_FEATURE_JARINPUTSOURCE)
    if(COLORER_USE_VCPKG)
        target_link_libraries(colorer_lib PUBLIC unofficial::minizip::minizip)
    else()
        target_link_libraries(colorer_lib PUBLIC minizip::minizip)
    endif()
endif()

if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
  target_link_libraries(colorer_lib PUBLIC stdc++fs)
endif()

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
  target_compile_options(colorer_lib PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${MAIN_FLAGS_CXX_DEBUG}>)
else()
  target_compile_options(colorer_lib PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${MAIN_FLAGS_CXX_RELEASE}>)
endif()

set_target_properties(colorer_lib PROPERTIES
    OUTPUT_NAME "colorer"
    VERSION ${COLORER_VERSION}
    SOVERSION ${COLORER_VERSION_MAJOR}
    DEBUG_POSTFIX "d"
    )
