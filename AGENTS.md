# Colorer library

C++ syntax-highlighting library. Languages are described in **HRC** (XML): prototypes, schemes, keywords, regexps, blocks, inherit/virtual. HRD maps regions to colors. Spec: `.agents/skills/colorer-hrc/hrc-ref.md`. Catalog overlays / `hrcsettings.xml`: `.agents/skills/colorer-hrc/overrides.md`. Site: https://colorer.github.io (sources often `../colorer.github.io`).

## Layout

- `src/colorer/` — library. Public headers at this level (`ParserFactory.h`, `HrcLibrary.h`, `TextParser.h`, `BaseEditor.h`, …); implementations in `parsers/`, `editor/`, `handlers/`, `xml/`, `cregexp/`, `io/`, `strings/icu/`, `strings/legacy/`.
- `src/colorer/common/spimpl.h` — pimpl (`ParserFactory::Impl`, `HrcLibrary::Impl`, `TextParser::Impl`).
- `src/colorer/common/Features.h.in` — CMake template. Generated `colorer/common/Features.h` is in the **build** tree (ICU, ZIP, deep trace), not under `src/`.
- `tools/colorer/` — CLI (`tools/colorer/ConsoleTools.cpp`).
- `tests/unit/` — Catch2 v3. Fixtures in `tests/unit/data/`.
- `tests/performance/` — `perftest` harness. Large input files in `tests/performance/samples/`.
- `.agents/skills/` — portable Agent Skills (`SKILL.md`); same layout as Colorer-schemes. HRC/CRegExp: `colorer-hrc` (`hrc-ref.md`, `overrides.md`, `core-parse.md`).

## Pipeline

`ParserFactory` loads `catalog.xml` → `HrcLibrary` (HRC) + HRD nodes. After that an app may overlay user HRC (`loadHrcPath`), `hrcsettings.xml` (`loadHrcSettings`), and user HRD (`loadHrdPath`). Do not edit the base catalog to customize types or params. Empty catalog path uses `COLORER_CATALOG`; empty user settings path with `user_defined=true` uses `COLORER_HRC_SETTINGS`. `TextParser` colors a `LineSource` into a `RegionHandler`. `BaseEditor` is the editor-facing API (`modifyLineEvent`, `idleJob`, `breakParse`). `CRegExp` (`src/colorer/cregexp/`) is Colorer’s regexp engine, not `std::regex`. XML goes through libxml2 (`src/colorer/xml/libxml2/`, `XmlReader`, `XmlInputSource`). `jar:` URIs require `COLORER_USE_ZIPINPUTSOURCE`.

HRC type load is **recursive**: `qualifyForeignName` and `<import>` call `loadFileType` while the parent’s `XMLNode` tree is still live. Do not collect imports, drop the parent tree, and reparse. Do not switch `LibXmlReader` to SAX: HRC splices other files through DTD `SYSTEM` entities; SAX dropped those children. Parse with a libxml DOM, copy into `XMLNode`, free the `xmlDoc` immediately. Do not preload types by scanning regexp/keyword text for `prefix:` QNames.

Several `ParserFactory` instances can exist in one process. Do not add process-global mutable parse, XML-load, or zip-cache state.

`idleJob` / `breakParse` are documented as usable from a background thread; the matcher is not internally synchronized.

## Build

C++17. ICU strings default ON. Linux can use system packages (`-DCOLORER_USE_VCPKG=OFF`). README uses Ninja; Unix Makefiles work if Ninja is missing. Release enables `-Werror`.

**Build directories (do not mix):**

- `out/build/` is the **user’s** tree (CMake presets: `out/build/<preset>`). Never configure, build, clean, delete, or write there. Never `rm -rf out/` — that would wipe the user’s builds.
- The **agent** builds under `out/`, in `out/agent/` only. Do not use `-B out` (that would make `out/build/` a subdirectory of the agent’s binary dir).

```bash
cmake -S . -B out/agent -G "Unix Makefiles" \
  -DCOLORER_USE_VCPKG=OFF -DCOLORER_BUILD_TEST=ON -DCMAKE_BUILD_TYPE=Release
cmake --build out/agent -j"$(nproc)"
```

Do not commit build trees (`out/`, `_build*`, `build*`, `cmake-build*`).

## Tests

Catch2 v3: run **tags separately**. Many tests open relative `data/` paths and need cwd `tests/unit`. New tests should resolve fixtures from `__FILE__` (see `tests/unit/test_baseeditor.cpp`).

```bash
cd tests/unit
../../out/agent/tests/unit/unit_tests '[cregexp]'
../../out/agent/tests/unit/unit_tests '[baseeditor]'
```

### Performance (`perftest`)

Harness: `out/agent/tests/perftest`. Corpus: `tests/performance/samples/` (`sqlite3.c`, PHP, Go, JS). Needs a Colorer-schemes catalog (`-b`).

```bash
./out/agent/tests/perftest -t5 -c5 \
  -b "$COLORER_SCHEMES_DIR/_build/base/catalog.xml" \
  -f tests/performance/samples/sqlite3.c
```

### Schemes catalog (Colorer-schemes)

Full-catalog load and golden coloring live in [Colorer-schemes](https://github.com/colorer/Colorer-schemes), not in Catch2. If this repo and Colorer-schemes are cloned as siblings under one parent (example: `…/src/Colorer-library` next to `…/src/Colorer-schemes`), `tests/schemes/run.sh` finds `../Colorer-schemes`. Otherwise set `COLORER_SCHEMES_DIR`. Run via `tests/schemes/run.sh` (links `out/agent/tools/colorer/colorer` into that tree's `bin/`). Agent instructions: `.agents/skills/colorer-schemes-tests/SKILL.md`.

`build.sh base` writes ordinary HRC/HRD files to `_build/base/`. `build.sh base.packed` writes a zip-packed catalog to `_build/base-packed/` (`jar:` URIs). **XML-load or zip/jar InputSource changes must run `load packed`.** Unpacked `load` does not cover that path. Packed builds need `-DCOLORER_USE_ZIPINPUTSOURCE=ON` and the `zip` tool.

```bash
./tests/schemes/run.sh load
./tests/schemes/run.sh load packed
./tests/schemes/run.sh parse --quick
```

Do not run full `parse` in the default agent loop. Do not vendor schemes or goldens into this repo.

## Strings and features

- `COLORER_USE_ICU_STRINGS` (default ON) → `src/colorer/strings/icu/`. OFF → `src/colorer/strings/legacy/` (`CString`, …).
- `COLORER_FEATURE_ZIPINPUTSOURCE` is set when `COLORER_USE_ZIPINPUTSOURCE` is ON.
- Encoding or ICU-sensitive changes must be checked on **both** string backends (`strings/icu/Encodings.cpp` and `strings/legacy/Encodings.cpp` when both exist).

## How to change this repo

- Changes must be **isolated**: one concern, files that belong only to that concern.
- Finish **one sufficient part**, then stop. Propose a commit **subject line** (English, imperative, why not what; `git log` style) and the complete file list. Do **not** `git commit` unless asked. Do **not** start the next part until the user says so.
- Match existing code: pimpl, `UnicodeString`, `COLORER_LOG_*`. No drive-by refactors. No extra markdown unless asked.
