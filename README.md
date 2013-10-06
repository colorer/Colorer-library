Colorer library
==========
  Colorer is a syntax highlighting library.
  
How to build library from source
----------
To build library from source, you will need:

  * Visual Studio 2010 or higher; or gcc 4.7
  * git
  * cmake 2.8.9 or higher

Download the source from git repository:

    cd src
    git clone https://github.com/colorer/Colorer-library.git --recursive

or update git repository:

    git pull
    git submodule update --recursive
    
From root path of repository call
    
    mkdir build
    cmake -G "Visual Studio 10" ../src
    colorer_lib.sln

Colorer tools
==========

How to build from source
----------

From root path of repository call
    
    mkdir build
    cmake -G "Visual Studio 10" ../tools/colorer
    consoletools.sln

Links
========================

* Project main page: [http://colorer.sourceforge.net/](http://colorer.sourceforge.net/)
* Colorer discussions (in Russian): [http://groups.google.com/group/colorer_ru](http://groups.google.com/group/colorer_ru)
* Colorer discussions (in English): [http://groups.google.com/group/colorer](http://groups.google.com/group/colorer)
