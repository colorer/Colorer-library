
all: xmlparser.exe


shared_path = ../../shared/
obj_output_slash = objs/
obj = obj

CPP = @cl
#DEBUG1 = -Zi
#DEBUG2 = /debug /fixed:no
DEBUG1 = /O2tiyb2 /wd4996 /W3
DEBUG2 =
DEFINES = -DCOLORER_FEATURE_JARINPUTSOURCE=0 -DCOLORER_FEATURE_HTTPINPUTSOURCE=0  -DUSE_DL_PREFIX  

CPPFLAGS = $(DEBUG1) /Fo"$(obj_output_slash)" -c -EHsc -I../../shared $(DEFINES)

OBJS = $(obj_output_slash)xmlparser.obj \
       $(obj_output_slash)UnicodeTools.obj \
       $(obj_output_slash)String.obj \
       $(obj_output_slash)Character.obj \
       $(obj_output_slash)DString.obj \
       $(obj_output_slash)SString.obj \
       $(obj_output_slash)StringBuffer.obj \
       $(obj_output_slash)MemoryChunks.obj \
       $(obj_output_slash)malloc.obj \
       $(obj_output_slash)Encodings.obj    \
       $(obj_output_slash)Exception.obj    \
       $(obj_output_slash)InputSource.obj  \
       $(obj_output_slash)FileInputSource.obj \
       $(obj_output_slash)xmldom.obj


include ../../shared/_dependencies.dep
include ../../shared/makefile.objs
include ../../shared/makefile.lines

xmlparser.exe: $(OBJS)
	link /OUT:xmlparser.exe $(OBJS) $(DEBUG2)

$(obj_output_slash)xmlparser.obj: xmlparser.cpp xmlprint.cpp
	$(CPP) $(CPPFLAGS) xmlparser.cpp

test: xmlparser.exe
	xmlparser.exe ../../../hrc/auto/gen/docbook.hrc >>t1
	xmlparser.exe ../../../hrc/base/perl.hrc >>t1
	xmlparser.exe tests/newB.ipl >>t1
	echo.>>t1
