#Colorer library
Colorer is a syntax highlighting library.
  
##How to build from source

### Main 
To build library and other utils from source, you will need:

  * Visual Studio 2019 or higher / gcc 9 or higher
  * git
  * cmake 3.15 or higher
  * [Conan](https://conan.io)

Download the source from git repository:

    git clone https://github.com/colorer/Colorer-library.git
 
From root path of repository call
    
    mkdir build
    conan install .. -s build_type=Release -s arch=x86_64 -s compiler.runtime=MT
    cmake -G "Visual Studio 16 2019" -DCOLORER_USE_CONAN=On ..
    colorer.sln

### Linux
You may build library on linux using standart package, not Conan.
Ubuntu example

    sudo apt install libicu-dev libxerces-c-dev libspdlog-dev libfmt-dev zlib1g-dev libminizip-dev
    mkdir build
    cmake ..
    cmake --build .

### Options for build
This options available for build

  * COLORER_INTERNAL_BUILD - This CMakeLists.txt called from other cmake project. Default 'OFF'.
  * COLORER_USE_CONAN - Use Conan for dependencies. Default 'OFF'.
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
