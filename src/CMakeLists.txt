cmake_minimum_required(VERSION 3.10)
project(colorer_lib CXX C)

#====================================================
# colorer
#====================================================
set(COLORER ./colorer)
set(EDITOR ./colorer/editor)
set(HANDLERS ./colorer/handlers)
set(PARSERS ./colorer/parsers)
set(VIEWER ./colorer/viewer)
set(COMMON ./colorer/common)
set(IO ./colorer/io)
set(REGEXP ./colorer/cregexp)
set(XML ./colorer/xml)
set(UNICODE ./colorer/unicode)

set(SRC_COLORER
    ${EDITOR}/BaseEditor.cpp ${EDITOR}/BaseEditor.h
    ${EDITOR}/EditorListener.h ${EDITOR}/OutlineItem.h
    ${EDITOR}/Outliner.cpp ${EDITOR}/Outliner.h
    ${EDITOR}/PairMatch.h
    ${HANDLERS}/LineRegionsSupport.cpp ${HANDLERS}/LineRegionsSupport.h
    ${HANDLERS}/LineRegionsCompactSupport.cpp ${HANDLERS}/LineRegionsCompactSupport.h
    ${HANDLERS}/RegionMapperImpl.cpp ${HANDLERS}/RegionMapperImpl.h
    ${HANDLERS}/StyledHRDMapper.h ${HANDLERS}/StyledHRDMapper.cpp
    ${HANDLERS}/TextHRDMapper.h ${HANDLERS}/TextHRDMapper.cpp
    ${HANDLERS}/LineRegion.h
    ${HANDLERS}/RegionDefine.h
    ${HANDLERS}/RegionMapper.h
    ${HANDLERS}/StyledRegion.h
    ${HANDLERS}/TextRegion.h
    ${PARSERS}/CatalogParser.cpp ${PARSERS}/CatalogParser.h
    ${PARSERS}/HRDNode.h
    ${PARSERS}/HRCParserImpl.cpp ${PARSERS}/HRCParserImpl.h
    ${PARSERS}/TextParserImpl.cpp ${PARSERS}/TextParserImpl.h
    ${PARSERS}/FileTypeImpl.h ${PARSERS}/FileTypeImpl.cpp
    ${PARSERS}/FileTypeChooser.h
    ${PARSERS}/KeywordList.h ${PARSERS}/KeywordList.cpp
    ${PARSERS}/SchemeImpl.h
    ${PARSERS}/TextParserHelpers.cpp ${PARSERS}/TextParserHelpers.h
    ${PARSERS}/ParserFactory.cpp
    ${PARSERS}/ParserFactory.h ${PARSERS}/ParserFactoryException.h
    ${PARSERS}/SchemeNode.cpp ${PARSERS}/SchemeNode.h
    ${PARSERS}/VirtualEntry.h
    ${VIEWER}/ParsedLineWriter.h
    ${VIEWER}/TextConsoleViewer.cpp ${VIEWER}/TextConsoleViewer.h
    ${VIEWER}/TextLinesStore.cpp ${VIEWER}/TextLinesStore.h
    ${COLORER}/Common.h
    ${COLORER}/Exception.h
    ${COLORER}/FileType.h
    ${COLORER}/HRCParser.h
    ${COLORER}/LineSource.h
    ${COLORER}/Region.h
    ${COLORER}/RegionHandler.h
    ${COLORER}/Scheme.h
    ${COLORER}/TextParser.h
    ${COMMON}/Colorer.h ${COMMON}/Colorer.cpp
    ${COMMON}/Exception.cpp
    ${COMMON}/Features.h
    ${IO}/FileInputSource.cpp ${IO}/FileInputSource.h
    ${IO}/FileWriter.h ${IO}/FileWriter.cpp
    ${IO}/InputSource.cpp ${IO}/InputSource.h
    ${IO}/JARInputSource.h ${IO}/JARInputSource.cpp
    ${IO}/SharedInputSource.h ${IO}/SharedInputSource.cpp
    ${IO}/Writer.cpp ${IO}/Writer.h
    ${IO}/HTTPInputSource.cpp ${IO}/HTTPInputSource.h
    ${IO}/MemoryFile.h ${IO}/MemoryFile.cpp
    ${IO}/StreamWriter.cpp ${IO}/StreamWriter.h
    ${REGEXP}/cregexp.cpp ${REGEXP}/cregexp.h
    ${XML}/XmlParserErrorHandler.cpp ${XML}/XmlParserErrorHandler.h
    ${XML}/XmlInputSource.cpp ${XML}/XmlInputSource.h
    ${XML}/BaseEntityResolver.cpp ${XML}/BaseEntityResolver.h
    ${XML}/LocalFileXmlInputSource.h ${XML}/LocalFileXmlInputSource.cpp
    ${XML}/SharedXmlInputSource.cpp ${XML}/SharedXmlInputSource.h
    ${XML}/ZipXmlInputSource.cpp ${XML}/ZipXmlInputSource.h
    ${XML}/XmlTagDefs.h
    ${XML}/XStr.cpp ${XML}/XStr.h
    ${UNICODE}/BitArray.cpp ${UNICODE}/BitArray.h
    ${UNICODE}/Character.h ${UNICODE}/Character.cpp
    ${UNICODE}/CharacterClass.cpp ${UNICODE}/CharacterClass.h
    ${UNICODE}/CString.cpp ${UNICODE}/CString.h
    ${UNICODE}/DString.h
    ${UNICODE}/Encodings.h ${UNICODE}/Encodings.cpp
    ${UNICODE}/String.cpp ${UNICODE}/String.h
    ${UNICODE}/SString.cpp ${UNICODE}/SString.h
    ${UNICODE}/UnicodeTools.cpp ${UNICODE}/UnicodeTools.h
    ${UNICODE}/x_charcategory.h ${UNICODE}/x_defines.h
    ${UNICODE}/x_charcategory2.h ${UNICODE}/x_encodings.h
    ${UNICODE}/x_charcategory_names.h ${UNICODE}/x_tables.h
    )

#====================================================
# common flags
#====================================================
set(COLORER_DEF -DNOGDI -DXERCES_STATIC_LIBRARY -DCOLORER_FEATURE_HTTPINPUTSOURCE=FALSE)
if(WIN32)
  set(COLORER_DEF ${COLORER_DEF} -D_WIN32)
endif(WIN32)
if(MSVC)
  set(COLORER_DEF ${COLORER_DEF} -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  # defaut for gcc
  # CMAKE_CXX_FLAGS=
  # CMAKE_CXX_FLAGS_DEBUG= -g
  # CMAKE_CXX_FLAGS_RELEASE= -O3 -DNDEBUG

  # set only new flags
  set(FLAGS_CXX_DEFAULT -fpermissive -Wall)
  set(FLAGS_RELEASE)
  set(FLAGS_DEBUG)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  # defaut for msvc
  # CMAKE_C_FLAGS= /DWIN32 /D_WINDOWS /W3
  # CMAKE_C_FLAGS_DEBUG= /MDd /Zi /Ob0 /Od /RTC1
  # CMAKE_C_FLAGS_RELEASE= /MD /O2 /Ob2 /DNDEBUG

  # CMAKE_CXX_FLAGS= /DWIN32 /D_WINDOWS /W3 /GR /EHsc
  # CMAKE_CXX_FLAGS_DEBUG= /MDd /Zi /Ob0 /Od /RTC1
  # CMAKE_CXX_FLAGS_RELEASE= /MD /O2 /Ob2 /DNDEBUG

  # set all flags and clean default, because Visual Studio project don`t overwrite default parameters
  # https://stackoverflow.com/questions/45995784/how-to-set-compiler-options-with-cmake-in-visual-studio-2017
  set(FLAGS_C_DEFAULT /DWIN32 /D_WINDOWS /W3)
  set(FLAGS_C_RELEASE /MT /O2 /Ob2 /DNDEBUG)
  set(FLAGS_C_DEBUG /MTd /Zi /Ob0 /Od /RTC1)
  set(FLAGS_CXX_DEFAULT /DWIN32 /D_WINDOWS /W3 /GR /EHsc)
  set(FLAGS_CXX_RELEASE /MT /O2 /Ob2 /DNDEBUG)
  set(FLAGS_CXX_DEBUG /MTd /Zi /Ob0 /Od /RTC1)

  set(CMAKE_C_FLAGS "")
  set(CMAKE_C_FLAGS_DEBUG "")
  set(CMAKE_C_FLAGS_RELEASE "")
  set(CMAKE_CXX_FLAGS "")
  set(CMAKE_CXX_FLAGS_RELEASE "")
  set(CMAKE_CXX_FLAGS_DEBUG "")
endif()

#====================================================
# build
#====================================================

add_library(colorer_lib STATIC ${SRC_COLORER} ${SRC_MALLOC})
target_include_directories(colorer_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ../libs/xercesc/src
    PUBLIC ${xerces-c_BINARY_DIR}/src
    PUBLIC ../libs/dlmalloc
    PUBLIC ../libs/spdlog/include
    )
target_link_libraries(colorer_lib
    PUBLIC xerces-c
    PUBLIC ZLIB::MINIZIP
    PUBLIC ${THREAD_LIBS}
    )
target_compile_definitions(colorer_lib PUBLIC ${COLORER_DEF})
set_target_properties(colorer_lib PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    )

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

set_target_properties(colorer_lib PROPERTIES OUTPUT_NAME "colorer")
if(MSVC)
  set_target_properties(colorer_lib PROPERTIES DEBUG_POSTFIX "d")
endif()

#====================================================
# test
#====================================================

# static analyze
if(COLORER_PVS_STUDIO_CHECK)
  pvs_studio_add_target(TARGET ${PROJECT_NAME}.analyze ALL
      ANALYZE ${PROJECT_NAME}
      MODE GA 64 OP CS
      CFG_TEXT "exclude-path=*/libs/*"
      OUTPUT FORMAT errorfile
      LOG ${PROJECT_NAME}.err)
endif()