# Coloring kernel (this library)

How a line becomes regions. HRC grammar is [hrc-ref.md](hrc-ref.md). This note is the **runtime hot path**: scheme search, `CRegExp`, `TextParser`, `BaseEditor`.

Code: `src/colorer/cregexp/cregexp.h`, `src/colorer/parsers/TextParserImpl.cpp`, `src/colorer/editor/BaseEditor.cpp`, plus `SchemeImpl` (`searchNodes` / `searchDispatch`) built in `HrcLibraryImpl::updateLinks`.

---

## Layers

```
editor events  →  BaseEditor  →  TextParser::parse / tryParseLine
                                      │
                                      │  LineSource (one UnicodeString per line)
                                      ▼
                               colorize()  ── per character gx ──►  searchMatch(scheme)
                                      │                                    │
                                      │                                    ├─ keywords (binary search)
                                      │                                    ├─ regexp  (CRegExp::parse)
                                      │                                    ├─ block   (start RE → nested colorize)
                                      │                                    └─ inherit / virtual
                                      ▼
                               RegionHandler  (BaseEditor forwards into a LineRegion ring)
```

`ParserFactory` loads `catalog.xml` → `HrcLibrary` (HRC schemes) + HRD. Several factories may exist in one process; there is no process-global parse/XML/zip cache. `TextParser::parse` / `tryParseLine` take a **shared** lock on that library so a concurrent `loadFileType` cannot mutate schemes under a live parse.

The matcher itself (`CRegExp::lowParse`) is **not** internally synchronized. `idleJob` / `breakParse` are documented as usable from a background thread; the editor must not run two parses of the same `TextParser` at once.

---

## 1. Scheme → ordered candidates (compile time)

HRC order is `SchemeImpl::nodes`. Runtime search does **not** walk that vector on every character.

After links resolve (`HrcLibrary::Impl::updateLinks`):

1. **`buildSearchNodes`** copies nodes into `searchNodes`. A static `<inherit>` (no `<virtual>`, scheme is not itself a virtual target) is **inlined**: the child's `searchNodes` are spliced in place. Cyclic inherit stays as `SNT_INHERIT`. Adjacent keyword lists with the same case/worddiv are **merged**.
2. **`searchDispatch`** (ASCII only, schemes with 2…65535 nodes): for each code point 0…127, a slice of `nodeIndexes` listing nodes that `canStartWith(ch)`. `searchMatch` uses that slice. Non-ASCII or no dispatch → full `searchNodes`. Dispatch is dropped if it is not smaller than ~¾ of a dense table (`indexes.size() * 4 <= nodes * 128 * 3`).

`CRegExp::canStartWith` / keyword `firstChar` feed that table. A nullable prefix (`^\s*`, `\b`, `[+-]?`) puts the node in **every** ASCII bucket — first-char filtering does not help those HRC patterns.

Virtual inherit (`<virtual scheme="A" subst="B"/>`) stays as `SNT_INHERIT`. `VTList` on the parse stack records active substitutions; `pushvirt` rewrites the scheme when the current inherit is a virtual target.

---

## 2. TextParser: one scheme, one position

`colorize(root_end_re, …)` is the inner loop. One scheme is active (`baseScheme`). Position in the line is `gx`. `schemeStart` is where the current block started (`~` in CRegExp).

Per line:

1. `LineSource::getLine` once (`clearLine` guard). Null line → stop (editor shutdown), no throw.
2. `CRegExp::collectAsciiChars` → `str_chars` (bits of ASCII present **anywhere** on the line). Passed into every `parse` / `mayMatch` on this line.
3. Try the **parent block's end-RE** (`root_end_re`) from `gx` to `len`. Miss → treat the window `[gx, min(gx+maxBlockSize, len)]` as the content bound (`matchend`).
4. `lowContentPriority`: content must not run past the parent end — `len` is clipped to `matchend.s[0]`.
5. From `gx` to that bound, `searchMatch`. Hit advances `gx`. Miss → `gx++`.
6. End-RE hit → return (leave the block). Else next line, or if `chunkLongLines` the next `maxBlockSize` window on the **same** line.

Default `chunkLongLines` is **false**: one window per line, rest skipped. That is the editor-safe behavior on minified/pathological lines. CLI/HTML coloring turns chunking on.

`maxBlockSize` (default 1000) also caps how far a child start-RE may look (`hiLen`).

### searchMatch

Pick the candidate slice, then first match wins (HRC order inside the slice):

| Node | Action |
|------|--------|
| keywords | Word/symbol table. ASCII first-char bitset + `asciiBegin`/`asciiEnd` range; then binary search. Lowercase copy of the line is built **only** when an ignore-case list is tried. Word bounds: default `\w` or `worddiv`. `indexOfShorter` retries a prefix keyword (`if` vs `ifdef`). |
| regexp | `mayMatch(gx, eol, schemeStart, str_chars)` then `parse`. Zero-length group 0 (`\m`…`\M` with no consume) is ignored so the loop does not stall. |
| block | Same start-RE check. On hit: `enterScheme`, recursive `colorize(end)`, `leaveScheme`. End-RE may use `\y`/`\Y` against the **start line**; that line is copied only if `hasBackTrace()`. |
| inherit | `VTList` substitution or recurse into `searchMatch(child)`. |

`priority="low"` on a regexp/block uses `lowLen` (parent end) as `eol` instead of `hiLen`. Do not “optimize” by ignoring that.

### ParseCache

Multi-line blocks are a tree (`sline`/`eline`, scheme, start-RE `SMatches`, optional `backLine`, virtual table snapshot). Single-line matches are not cached. Siblings are ordered by `sline` and do not overlap. `searchLine` keeps a per-parent cursor (`search_child`) so forward `idleJob` chunks do not rescan the whole list; a long jump toward the start restarts from the head.

`parse(from, num, mode)`:

- `TPM_CACHE_OFF` — from the root scheme, ignore cache.
- `TPM_CACHE_READ` — jump to the cache node covering `from`, color without rewriting the tree.
- `TPM_CACHE_UPDATE` — drop children from `from` onward, rebuild while coloring.

Returning from a nested block that spans lines inserts/updates a cache node. Same-line empty blocks are discarded (`empty-block.test`).

### tryParseLine (incremental edit)

`BaseEditor::modifyLineEvent` calls this instead of invalidating the rest of the file.

1. Snapshot the open-block stack that **cache already predicts** for `line+1`.
2. Reparse `line` with `TPM_CACHE_READ` while recording the stack (`tracingTry`).
3. If the stack (scheme, block node, start-RE captures, and start-line text when the end-RE has backtrace) matches the prediction, the rest of the file stays valid. Otherwise `invalidLine = line` like `modifyEvent`.

---

## 3. CRegExp: compile → filters → NFA

Not `std::regex`. Unicode as 16-bit units. Colorer extras (`COLORERMODE`): `~` scheme start, `\m`/`\M` group-0 bounds, `\y`/`\Y` backtrace into another RE's captures.

Tree: `SRegInfo` linked list (`next` = concatenation, `un.param` = group/quantifier/left `|` branch). Max 16 numbered + 16 named captures; further groups compile as non-capturing so old HRC still loads.

### optimize() (once per setRE)

| Fact | Used to skip |
|------|----------------|
| `firstCharMask` | First **consuming** ASCII set. Useless if the prefix is nullable or the mask is “all 128”. |
| `firstNode` | First literal/class/word for `quickCheck`. |
| `startAnchor` | Pattern begins with `^` (not `/m`) or `~`. Only `pos==0` / `pos==schemeStart` can match, even with `positionMoves`. |
| `endAnchor` + `maxLen` | Pattern ends with `$` (not `/m`, no top-level `\|`). Match cannot start more than `maxLen` before `eol`. `maxLen==-1` if `*`/`+`/`\N`/`\y` make length unbounded. |
| `requiredChars[]` | Up to 4 ASCII sets that **must** appear somewhere in the subject. `/i` letters are omitted (non-ASCII case folds). Nullable prefixes do not contribute. |
| `ReOr.branchFirst` | Per-alternative first-char mask. Skip the left branch in `lowParse` when the current ASCII char cannot start it. Left unused if the branch is nullable or starts with `\m`/`\M`/`\b`/lookaround/`^`/`$` — those still have side effects (`\M` bounds group 0 for a later alternative). |

`$` is `toParse == eol` (the `eol` argument, not necessarily `str->length()`). `/m` disables both start and end anchors.

### parse() / parseRE()

1. `bindSubject` pins `parseBuf = str->getBuffer()` so the NFA does not index `UnicodeString` per step.
2. Required-char ∩ `subjectChars` empty → fail (no NFA).
3. Start-anchor mismatch → fail.
4. End-anchor: if `positionMoves` and not start-anchored, jump `toParse` to `eol - maxLen`; if still `eol - toParse > maxLen` → fail.
5. Fixed position: first-char mask + `quickCheck`.
6. Moving search (`positionMoves`, typical for HRC `start`/`match`): at each offset, skip via first-char / `quickCheck`; start-anchor still fails the whole parse rather than sliding.

Each attempt resets `\m`/`\M` and all capture slots so a failed offset cannot leak.

`mayMatch(pos, eol, soscheme, subjectChars)` is the same cheap tests **without** running the NFA. `TextParser` calls it before `parse` for start-RE, end-RE, and content regexp.

### lowParse

Explicit backtracking stack (`insert_stack` / `check_stack`), not C++ recursion. `RegExpStack` is process-wide and **reused** across all `CRegExp`; `count_elem` is reset each `parseRE`. Do not parse concurrently. Do not clear it from `ParserFactory` teardown — a short-lived probe factory used to wipe a live editor parse on another object.

`parseStepLimit` (default 1e6) counts NFA steps in one `parse()`. Exceeded → match fails (not a wall-clock quantum). Pathological HRC (`FuncOutline`-style lazy overlap × `[^;]*`) hits this; the right fix is usually the scheme, not raising the limit.

Helpers on the hot path (`insert_stack`, `check_stack`, `checkMetaSymbol`, `isWordBoundary`) are inline in the header.

---

## 4. BaseEditor: window, invalidation, idle

`BaseEditor` is the editor-facing `RegionHandler`. It owns a `TextParser`, maps HRC regions through HRD, and stores colored lines in a **ring** (`LineRegionsSupport`).

| Field | Role |
|-------|------|
| `wStart`, `wSize` | Visible window (`visibleTextEvent`). Ctor default `wSize=20`. |
| `invalidLine` | First line whose coloring (or cache) is stale. `haveInvalidLine()` is `invalidLine < lineCount`. |
| `lrSize` | Ring capacity. **Never shrinks.** Grows to `3 * wSize` when the window grows. |
| `backParse` | If `getLineRegions(lno)` would need more than this many lines from `invalidLine`, return null (caller should `validate`). |

Layout math (do not “simplify” — `-ht` goldens encode it):

- Visible window = `wSize`.
- Parse/wrap cover = `2 * wSize` (first `getLineRegions` at default size colors 40 lines).
- Ring = `3 * wSize` so a small resize (20→21) fits without realloc, and idle warming does not move the visible slice.

`validate(lno, rebuildRegions)`:

- `rebuildRegions == false` (`idleJob`): parse `[invalidLine, lno]` with `TPM_CACHE_UPDATE` only. **Does not** move `wStart` or the LineRegion ring. Used to warm `ParseCache` in the background.
- `rebuildRegions == true` (`getLineRegions`): if the visible window left the ring’s `firstLine` cover, snap `firstLine` to a multiple of `wSize` and parse that cover. Else if `invalidLine` is inside the cover, parse from `invalidLine` with `TPM_CACHE_UPDATE`. `invalidLine` advances to `stopLine+1` only on UPDATE.

`modifyEvent(top)` sets `invalidLine = min(invalidLine, top)` and notifies listeners.

`modifyLineEvent(line)`: if `invalidLine > line` and `tryParseLine(line)` succeeds, **leave `invalidLine` alone**. Else `invalidLine = line`.

`idleJob(time)`: `validate(invalidLine + 100 + 4*time, false)` with `time` clamped to 0…100.

Pair search walks `LineRegion` lists looking for `def:PairStart` / `def:PairEnd`. Local pair stays in the visible range; global uses `[0, lineCount)` and can force a large `validate`.

---

## 5. What not to break

- **Match order** inside a scheme (and thus goldens in Colorer-schemes). Filters may only skip attempts that cannot succeed.
- **`$` vs `eol`**, `~` vs `schemeStart`, `\m`/`\M` isolation per offset, `\y` needing the start-line copy.
- **`priority="low"`** and virtual inherit (do not flatten those into `searchNodes`).
- **Recursive `loadFileType`** while the parent HRC `XMLNode` tree is live; no SAX for HRC (DTD `SYSTEM` entities). See [hrc-ref.md](hrc-ref.md#xml-load-this-library).
- **Both string backends** (`COLORER_USE_ICU_STRINGS` on/off) for anything that touches `UnicodeString` / `parseBuf`.
- **No process-global** mutable parse, XML-load, or zip cache. CRegExp stack is per-thread, not per-factory.

Checks: `[cregexp]`, `[textparser]`, `[baseeditor]` on ICU and legacy; `./tests/schemes/run.sh parse --quick` then full `parse` (0 changed). XML/zip changes also need `load` and `load packed`. Large-file timing: `tests/performance/samples/` via `perftest -t5`.
