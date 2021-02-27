# Colorer library
Colorer is a syntax highlighting library. 
[![build](https://github.com/colorer/Colorer-library/workflows/build/badge.svg)](https://github.com/colorer/Colorer-library/actions?query=workflow%3A%22build%22)
  
## How to build from source

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

Build colorer and dependency, if they are not in the local cache:
    
    cd c:\work\colorer-library
    mkdir build
    cd build
    cmake -S .. -G "Visual Studio 16 2019" -DCMAKE_TOOLCHAIN_FILE=C:/Work/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static -DVCPKG_OVERLAY_PORTS=..\external\vcpkg-ports -DVCPKG_FEATURE_FLAGS=versions
    colorer.sln

For x86 platform use `--triplet=x86-windows-static`.
Once builded, the dependencies will be cached in the local cache.

### Linux
You may build library on linux using standart package, without vcpkg.
Ubuntu example

    sudo apt install libicu-dev libxerces-c-dev libspdlog-dev libfmt-dev libminizip-dev
    git clone https://github.com/colorer/Colorer-library.git
    cd Colorer-library
    mkdir build
    cd build
    cmake ..
    cmake --build .

### Options for build
This options available for build

  * `COLORER_INTERNAL_BUILD` - This CMakeLists.txt called from other cmake project. Default 'OFF'.
  * `COLORER_USE_VCPKG` - Use dependencies installed via vcpkg. Default 'ON'.
  * `COLORER_BUILD_ARCH` - Build architecture. Default 'x64'.
  * `COLORER_BUILD_TOOLS` - Build colorer tools. Default 'ON'.
  * `COLORER_BUILD_TEST` - Build tests. Default 'OFF'.
  * `COLORER_PVS_STUDIO_CHECK` - Check project in PVS-Studio. Default 'OFF'.
  * `COLORER_USE_HTTPINPUTSOURCE` - Use http inputsource, only Windows. Default 'OFF'. 
  * `COLORER_USE_JARINPUTSOURCE` - Use jar inputsource for schemes. Default 'ON'.
  * `COLORER_USE_DEEPTRACE` - Use trace logging. Default 'OFF'.

Links
========================

* Project main page: [http://colorer.sourceforge.net/](http://colorer.sourceforge.net/)
* Colorer discussions (in Russian): [http://groups.google.com/group/colorer_ru](http://groups.google.com/group/colorer_ru)
* Colorer discussions (in English): [http://groups.google.com/group/colorer](http://groups.google.com/group/colorer)
