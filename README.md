#Colorer library
Colorer is a syntax highlighting library.
  
##How to build from source

### Main 
To build library and other utils from source, you will need:

  * Visual Studio 2019 or higher / gcc 9 or higher
  * git
  * cmake 3.15 or higher
  * [vcpkg](https://github.com/microsoft/vcpkg)

### Windows
Install [vcpkg](https://github.com/microsoft/vcpkg), for example, in c:\work\vcpkg:

    git clone https://github.com/microsoft/vcpkg
    .\vcpkg\bootstrap-vcpkg.bat

Download the source of Colorer, for example, in c:\work\colorer-library:

    git clone https://github.com/colorer/Colorer-library.git colorer-library
 
Build dependencies:

    c:\work\vcpkg\vcpkg install minizip spdlog icu[data-archive] xerces-c --triplet=x64-windows-static --overlay-ports=c:\work\colorer-library\external\vcpkg-ports

For x86 platform use `--triplet=x86-windows-static`.
Once builded, the dependencies will be cached in the system.
Build colorer:
    
    cd c:\work\colorer-library
    mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:/work/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static -G "Visual Studio 16 2019"
    colorer.sln

### Linux
You may build library on linux using standart package, without vcpkg.
Ubuntu example

    sudo apt install libicu-dev libxerces-c-dev libspdlog-dev libfmt-dev zlib1g-dev libminizip-dev
    git clone https://github.com/colorer/Colorer-library.git
    cd Colorer-library
    mkdir build
    cd build
    cmake ..
    cmake --build .

### Options for build
This options available for build

  * COLORER_INTERNAL_BUILD - This CMakeLists.txt called from other cmake project. Default 'OFF'.
  * COLORER_BUILD_WITHOUTVCPKG - Build without vcpkg, for example on linux. Default 'OFF'.
  * COLORER_WINx86 - Set on if building for Windows x86. Default 'OFF'.
  * COLORER_TOOLS_BUILD - Build colorer tools. Default 'ON'.
  * COLORER_BUILD_TEST - Set on to build test. Default 'OFF'.
  * COLORER_PVS_STUDIO_CHECK - Check project in PVS-Studio. Default 'OFF'.


  * COLORER_USE_HTTPINPUTSOURCE - Use http inputsource, only Windows. Default 'OFF'. 
  * COLORER_USE_JARINPUTSOURCE - Use jar inputsource for schemes. Default 'ON'.
  * COLORER_USE_DEEPTRACE - Use trace logging. Default 'OFF'.

Links
========================

* Project main page: [http://colorer.sourceforge.net/](http://colorer.sourceforge.net/)
* Colorer discussions (in Russian): [http://groups.google.com/group/colorer_ru](http://groups.google.com/group/colorer_ru)
* Colorer discussions (in English): [http://groups.google.com/group/colorer](http://groups.google.com/group/colorer)
