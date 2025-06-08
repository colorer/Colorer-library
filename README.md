# Colorer library

Colorer is a syntax highlighting library.
[![build](https://github.com/colorer/Colorer-library/actions/workflows/colorer_ci.yml/badge.svg)](https://github.com/colorer/Colorer-library/actions/workflows/colorer_ci.yml)

## Documentation

https://colorer.github.io
https://github.com/colorer/colorer.github.io

## How to build from source

### Main

To build library and other utils from source, you will need:

* Visual Studio 2019 / gcc 7 / clang 7 or higher
* git
* cmake 3.10 or higher
* vcpkg - required for Windows, optional for linux

### Windows

Download the source of Colorer:

```bash
git clone https://github.com/colorer/Colorer-library.git
```

Setup vcpkg, or use the preset (set env VCPKG_ROOT)

```bash
git clone https://github.com/microsoft/vcpkg.git
set VCPKG_ROOT=<path_to_vcpkg>
%VCPKG_ROOT%\bootstrap-vcpkg.bat -disableMetrics
```

#### IDE

Open folder with Colorer-library from IDE like as Clion, VSCode, VisualStudio.
In the CMakePresets file.json configurations have been created for standard builds.

#### Console

Setup Visual Studio environment (x64 or x86/arm64 for platforms)

```bash
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
```

Build colorer and dependency, if they are not in the local cache:

```bash

mkdir build
cd build
cmake -S .. -G "NMake Makefiles" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-win-static -DCOLORER_BUILD_ARCH=x64
cmake --build .
```

For x86 platform use `-DVCPKG_TARGET_TRIPLET=x86-win-static -DCOLORER_BUILD_ARCH=x86`, arm64 - `-DVCPKG_TARGET_TRIPLET=arm64-win-static -DCOLORER_BUILD_ARCH=arm64`.

Once built, the dependencies will be cached in the local cache.

### Linux

You may build library on linux using standard package, without vcpkg.

#### Ubuntu example

```bash
sudo apt install libicu-dev zlib1g-dev libminizip-dev libxml2-dev
git clone https://github.com/colorer/Colorer-library.git
cd Colorer-library
mkdir _build
cmake -S . -B _build -G "Ninja" -DCOLORER_USE_VCPKG=OFF
cmake --build _build -j$(nproc --all)
```

#### CentOS Example

```bash
sudo yum install libicu-devel zlib-devel minizip1.2-devel
git clone https://github.com/colorer/Colorer-library.git
cd Colorer-library
mkdir _build
cmake -S . -B _build -G "Ninja" -DCOLORER_USE_VCPKG=OFF
cmake --build _build -j$(nproc --all)
```
### MacOS

```bash
brew install icu4c minizip ninja libxml2
git clone https://github.com/colorer/Colorer-library.git
cd Colorer-library
mkdir _build
cmake -S . -B _build -G "Ninja" -DCOLORER_USE_VCPKG=OFF
cmake --build _build -j$(getconf _NPROCESSORS_ONLN)
```

### Options for build

This options available for build

* `COLORER_USE_VCPKG` - Use dependencies installed via vcpkg. Default 'ON'.
* `COLORER_BUILD_ARCH` - Build architecture. Default 'x64'.
* `COLORER_BUILD_TOOLS` - Build colorer tools. Default 'ON'.
* `COLORER_BUILD_TEST` - Build tests. Default 'OFF'.
* `COLORER_BUILD_INSTALL` - Make targets for install. Default 'ON'.
* `COLORER_BUILD_HARD_WARNINGS` - Compiler warnings as error on Release build. Default 'ON'.
* `COLORER_BUILD_OLD_COMPILERS` - Use own implementation for standard library. Default 'OFF'.
* `COLORER_USE_ZIPINPUTSOURCE` - Enable the ability to work with schemes in zip archives. Default 'ON'.
* `COLORER_USE_DEEPTRACE` - Use trace logging. Default 'OFF'.
* `COLORER_USE_ICU_STRINGS` - Use ICU library for strings. Default 'ON'.

Links
========================

* Project main page (older): [http://colorer.sourceforge.net/](http://colorer.sourceforge.net/)
* Colorer github discussions: [https://github.com/colorer/Colorer-library/discussions](https://github.com/colorer/Colorer-library/discussions)
* Colorer discussions (in Russian): [http://groups.google.com/group/colorer_ru](http://groups.google.com/group/colorer_ru)
* Colorer discussions (in English): [http://groups.google.com/group/colorer](http://groups.google.com/group/colorer)
