cmake_minimum_required (VERSION 2.8.9)
project (colorer_lib)

#====================================================
# init variables
#====================================================

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()

#====================================================
# colorer
#====================================================
IF (NOT COLORER_FEATURE_HTTPINPUTSOURCE)
  add_definitions(-DCOLORER_FEATURE_HTTPINPUTSOURCE=FALSE)
ENDIF()

set(COLORER ./colorer)
set(EDITOR ${COLORER}/editor)
set(HANDLERS ${COLORER}/handlers)
set(PARSERS ${COLORER}/parsers)
set(VIEWER ${COLORER}/viewer)
set(COMMON ./common)
set(REGEXP ./cregexp)
set(XML ./xml)
set(UNICODE ./unicode)

set(SRC_COLORER
   ${EDITOR}/BaseEditor.cpp ${EDITOR}/BaseEditor.h
   ${EDITOR}/EditorListener.h ${EDITOR}/OutlineItem.h 
   ${EDITOR}/Outliner.cpp ${EDITOR}/Outliner.h
   ${EDITOR}/PairMatch.h
   ${HANDLERS}/DefaultErrorHandler.h
   ${HANDLERS}/ErrorHandlerWriter.cpp ${HANDLERS}/ErrorHandlerWriter.h
   ${HANDLERS}/FileErrorHandler.cpp ${HANDLERS}/FileErrorHandler.h
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
   ${PARSERS}/HRCParserImpl.cpp ${PARSERS}/HRCParserImpl.h
   ${PARSERS}/TextParserImpl.cpp ${PARSERS}/TextParserImpl.h
   ${PARSERS}/helpers/FileTypeImpl.h 
   ${PARSERS}/helpers/HRCParserHelpers.cpp ${PARSERS}/helpers/HRCParserHelpers.h
   ${PARSERS}/helpers/TextParserHelpers.cpp ${PARSERS}/helpers/TextParserHelpers.h
   ${VIEWER}/ConsoleTools.cpp ${VIEWER}/ConsoleTools.h
   ${VIEWER}/ParsedLineWriter.h
   ${VIEWER}/TextConsoleViewer.cpp ${VIEWER}/TextConsoleViewer.h
   ${VIEWER}/TextLinesStore.cpp ${VIEWER}/TextLinesStore.h
   ${COLORER}/ErrorHandler.h
   ${COLORER}/HRCParser.h 
   ${COLORER}/ParserFactory.cpp ${COLORER}/ParserFactory.h ${COLORER}/ParserFactoryException.h
   ${COLORER}/RegionHandler.h
   ${COLORER}/TextParser.h
   ${COLORER}/FileType.h
   ${COLORER}/LineSource.h
   ${COLORER}/Region.h
   ${COLORER}/Scheme.h
   ${COMMON}/Common.h
   ${COMMON}/Exception.h ${COMMON}/Exception.cpp
   ${COMMON}/Hashtable.h ${COMMON}/HashtableCore.h
   ${COMMON}/Logging.cpp ${COMMON}/Logging.h
   ${COMMON}/MemoryChunks.cpp ${COMMON}/MemoryChunks.h ${COMMON}/MemoryOperator.h
   ${COMMON}/Features.h
   ${COMMON}/Vector.h
   ${COMMON}/io/FileInputSource.cpp ${COMMON}/io/FileInputSource.h
   ${COMMON}/io/FileWriter.h ${COMMON}/io/FileWriter.cpp
   ${COMMON}/io/InputSource.cpp ${COMMON}/io/InputSource.h
   ${COMMON}/io/JARInputSource.h ${COMMON}/io/JARInputSource.cpp
   ${COMMON}/io/SharedInputSource.h ${COMMON}/io/SharedInputSource.cpp
   ${COMMON}/io/Writer.cpp ${COMMON}/io/Writer.h
   ${COMMON}/io/HTTPInputSource.cpp ${COMMON}/io/HTTPInputSource.h
   ${COMMON}/io/MemoryFile.h ${COMMON}/io/MemoryFile.cpp 
   ${COMMON}/io/StreamWriter.cpp ${COMMON}/io/StreamWriter.h
   ${REGEXP}/cregexp.cpp ${REGEXP}/cregexp.h
   ${XML}/xmldom.cpp ${XML}/xmldom.h
   ${UNICODE}/BitArray.cpp ${UNICODE}/BitArray.h
   ${UNICODE}/Character.h ${UNICODE}/Character.cpp
   ${UNICODE}/CharacterClass.cpp ${UNICODE}/CharacterClass.h
   ${UNICODE}/DString.cpp ${UNICODE}/DString.h
   ${UNICODE}/Encodings.h ${UNICODE}/Encodings.cpp
   ${UNICODE}/String.cpp ${UNICODE}/String.h
   ${UNICODE}/StringBuffer.h ${UNICODE}/StringBuffer.cpp
   ${UNICODE}/SString.cpp ${UNICODE}/SString.h
   ${UNICODE}/UnicodeTools.cpp ${UNICODE}/UnicodeTools.h
   ${UNICODE}/x_charcategory.h ${UNICODE}/x_defines.h
   ${UNICODE}/x_charcategory2.h ${UNICODE}/x_encodings.h
   ${UNICODE}/x_charcategory_names.h ${UNICODE}/x_tables.h
)

# add ${XML} to fix build on farmanager server
include_directories (./ ${XML})

#====================================================
# zlib
#====================================================
set(ZLIB ../libs/zlib)
set(MINIZIP ${ZLIB}/contrib/minizip)

set(SRC_ZLIB
    ${ZLIB}/adler32.c ${ZLIB}/crc32.c ${ZLIB}/crc32.h ${ZLIB}/inffast.c ${ZLIB}/inffast.h
    ${ZLIB}/inflate.c ${ZLIB}/inflate.h ${ZLIB}/inftrees.c ${ZLIB}/inftrees.h 
    ${ZLIB}/uncompr.c ${ZLIB}/zutil.c ${ZLIB}/zutil.h
    ${MINIZIP}/ioapi.h ${MINIZIP}/ioapi.c ${MINIZIP}/minizip.c ${MINIZIP}/unzip.c ${MINIZIP}/unzip.h
)

# add ${MINIZIP} to fix build on farmanager server
include_directories (../libs/zlib/ ${MINIZIP})

#====================================================
# dlmalloc
#====================================================
set(DLMALLOC ../libs/dlmalloc)
set(SRC_MALLOC ${DLMALLOC}/dlmalloc.c)
                          
include_directories (../libs/dlmalloc)
add_definitions(-DUSE_DLMALLOC -DUSE_DL_PREFIX)

#====================================================
# common flags
#====================================================

set(FLAGS_RELEASE "-U_DEBUG -DNDEBUG -D_UNICODE -DUNICODE")
set(FLAGS_DEBUG "-D_DEBUG -DDEBUG -D_UNICODE -DUNICODE")

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)

  set(COLORER_WARNING_FLAGS "")
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} /EHsc ")

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

set(SRC_FILES ${SRC_COLORER} ${SRC_ZLIB} ${SRC_MALLOC})
add_library(colorer_lib STATIC ${SRC_FILES})
