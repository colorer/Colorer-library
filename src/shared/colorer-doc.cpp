
/** @addtogroup unicode Unicode core
      Unicode abstraction layer.
      Unicode classes creates common abstraction
      of unicode strings in colorer library.
      Basically, all defined string classes can't modify
      their content (except StringBuffer class), and serves
      as unicode wrappers over different data sources.
      SString is a static wrapper, it does not depends
      on any external source, and stores string chars in
      an internal buffer.
      DString is a dynamic wrapper, which serves only as a shell
      for the external character data. It can wraps over characters
      in any single byte enconding, or over any of unicode encondings.

      All character information can be accessed through the Character
      class and it's static methods. All of them work with unicode
      character in two-byte form (wchar). Note, that surrogates are not supported.
      All character properties are stored internally in
      two-stage tables, generated from Unicode database by perl
      scripts.
*/

/** @addtogroup common Common interfaces
      Common classes, exception and service classes.
      Common abstract primitives, basic definitions,
      global compilation ruling definitions.
*/

/** @addtogroup common_io IO core interfaces
      @ingroup common
      Classes and interfaces, which implements an abstraction of input-output layer.
      These includes base InputSource interface, which is extended by
      couple of classes, used to provide access to different URL schemas.
*/

/**
    @addtogroup colorer Colorer interfaces
      Basic colorer interfaces and abstract classes.
      All the commons, used in Colorer parser working.
*/

/**
    @addtogroup colorer_editor Editor
      @ingroup colorer
      Common editor abstraction classes.
      Implements most client's editor functionality, linked with
      colorer library parser's calls and processing.
      Serves as facade for the complex relations of the internal
      parsing objects.
*/

/**
    @addtogroup colorer_handlers Handlers
      @ingroup colorer
      Different service handlers classes and interfaces.
      They are used to recieve and store an information, passed
      from text parsers.
*/

/**
    @addtogroup colorer_parsers Parsers
      @ingroup colorer
      Common parsers implementation classes.
      These includes HRCParserImpl, TextParserImpl and their
      helpers classes,  used to implement core parse process.
*/

/**
    @addtogroup colorer_viewer Viewer/streaming
      @ingroup colorer
      Classes, which allows simple text parse and stream
      processing of parsed results. Also contains some
      static service methods and classes, useful in common jobs.
*/

/** @addtogroup far_plugin FAR Manager plugin
      Implementation of FAR plugin interface.
      Works with FAR 1.65-1.70bx.
      Implements most of library functionality, including
      text coloring, outlining, pairs search.
*/



/**
@mainpage

  <a href='../index.html'>Documentation home</a>

*/


/**
  @page indexes Indexes

  @par
  Here you'll find indexes of all colorer library classes, structures and files symbols.

  - <a href="functions.html">Members Index</a>

*/

/*
  - <a href="files.html">File List</a>
  - <a href="globals.html">File Members Index</a>
*/

/**
  @page dependencies Dependencies generation

  @par
  In order, to compile the library, you have to include a dependencies file.
  By default, this file is already generated and included into a making process.

  @par
  But, if you've changed structure of source codes, or made other changes, which
  create new/delete old dependencies, you have to regenerate dependencies
  file (/src/shared/_dependencies.dep)

  @par
  You can do this simply with
  <code>make -f _makefile.deps</code> call.
  This command runs gcc in precompilation and dependencies generation mode.
  After this perl script links all generated *.d files into one <code>_dependencies.dep</code>,
  and you can start your standard compilation process.

*/

/**
  @page samples Code Samples

  @section basic Basic library usage

  @par To parse text you must create:
    - Class, acting as source of text lines (implements LineSource interface).
    - ParserFactory instance, which stores all global parse data and structures.
    - BaseEditor class for each editor (or your colorized object).

  @code

    // Source file text lines store.
    TextLinesStore textLinesStore;
    textLinesStore.loadFile(inputFileName, inputEncoding, true);
    // parsers factory
    ParserFactory pf(catalogPath);
    // Base editor to make primary parse
    BaseEditor baseEditor(&pf, &textLinesStore);
    baseEditor.setRegionMapper(&DString("rgb"), &DString("default"));
    baseEditor.setFileType(DString("cpp"));
    // Initial line count notify
    baseEditor.lineCountEvent(textLinesStore.getLineCount());
    // Iterate over text lines
    for(int idx = 0; idx < textLinesStore.getLineCount(); idx++){
      LineRegions *lr = baseEditor.getLineRegions(idx);
      // Iterate over line regions
      for(LineRegions *next = lr; next != null; lr = lr->next){
        // work with this region:
        //   next->start
        //   next->end
        //   next->region
        StyledRegion *sr = next->styled();
        // colors:
        //   sr->fore, sr->back
      };
    };
  @endcode
*/


/**
  @page acknowledgements Acknowledgements

  @section hrcdatabase HRC database
  @par
  I'm working on maintenance of the HRC database with help of
  many people. You can find their acknowledgements in corresponding
  HRC file headers. Most 'named' HRC's has their author's contact
  email or url.

  @section unzip Unzip code
  @par
  Colorer library uses minizip source codes, written by Gilles Vollant and based on
  <a href='mailto:jloup@gzip.org'>Jean-loup Gailly</a> and <a href='mailto:madler@alumni.caltech.edu'>Mark Adler</a>
  zlib library.

  @par
  You can find latest minizip source codes on
  <a href='http://www.winimage.com/zLibDll/unzip.html'>http://www.winimage.com/zLibDll/unzip.html</a>,
  or in zlib distribution package
  <a href='http://www.gzip.org/zlib/zlib114.zip'>http://www.gzip.org/zlib/zlib114.zip</a>.

  @par
  Colorer uses minizip v0.20, found at
  <a href='http://www.winimage.com/zLibDll/unzip20.zip'>http://www.winimage.com/zLibDll/unzip20.zip</a>.

  @section dtd2xsd dtd2xsd.pl
  @par
  Colorer package includes dtd2xsd script, used for transformation
  of DTD document descriptions into XML Schema form.
  This perl script is written by
  <a href='mailto:holstege@mathling.com'>Mary Holstege</a>, Yuichi Koike, Dan Connolly, <a href='mailto:bert@w3.org'>Bert Bos</a>.
  @par
  I've modified it and fixed some minor errors and problems.

*/