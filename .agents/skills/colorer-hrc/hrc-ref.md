# HRC / HRD / catalog

Agent spec for this library. Engine is `CRegExp` (`src/colorer/cregexp/`), not `std::regex`. Tag names: `src/colorer/base/XmlTagDefs.h`.

## Runtime

`ParserFactory` loads `catalog.xml` → HRC (`HrcLibrary`) + HRD. After catalog, apps overlay user HRC (`loadHrcPath`), `hrcsettings.xml` (`loadHrcSettings`), user HRD (`loadHrdPath`) — [overrides.md](overrides.md). File type: sum `<filename>` / `<firstline>` weights; first max wins. Type XML loads lazily when selected. Parse starts at the scheme whose `name` equals the type name (packages need no base scheme). One scheme is active at a time. `TextParser` emits regions + enter/leave scheme to `RegionHandler`.

Coloring hot path (scheme first-char lists, CRegExp skip filters, ParseCache, BaseEditor window/invalidation): [core-parse.md](core-parse.md).

URI on `location/@link`: relative to the parent resource, or absolute. Missing scheme → `file://`. `jar:archive!path` needs `COLORER_USE_ZIPINPUTSOURCE`. Packed catalogs rewrite HRC `link` to `jar:…`.

## XML load (this library)

`LibXmlReader` uses libxml `xmlCtxtReadFile` (DOM, `XML_PARSE_NOENT | XML_PARSE_NONET`), copies the tree into `XMLNode`, then **frees the `xmlDoc`**. `parseHRC` / catalog / HRD walk `XMLNode` only.

**Do not switch this path to SAX** (or parse HRC directly from `xmlNode*` without a C++ tree, if that skips entity expansion). Many HRC files splice extra elements through DTD general `SYSTEM` entities, for example `xpath.hrc` (`&xpath-internal-hack;`, `&xpath-xml;`), `far.hrc`, `perl.hrc`. libxml inserts those children into the DOM; SAX building `XMLNode` did not, so schemes were missing and `./tests/schemes/run.sh load` failed. Catch2 fixture: `tests/unit/data/type_entity_incl.hrc` + `type_entity_frag.hrc`. Catalog `env:` / `jar:` entities stay on the existing `xmlMyExternalEntityLoader`.

Type bodies load **recursively**. `qualifyForeignName` (`type:name`) and `<import type="…">` call `loadFileType` while the **parent `XMLNode` is still on the stack**. That is required:

- `type_loading` is set in `addType`, before `parseTypeBlock`. Nested loads of specializations such as `inherit scheme="svg-css:css"` from type `css` need the parent already `type_loading` with regions defined. Preloading the extension first resolves `css:PropertyName` as a missing name.
- Cycles (css ↔ svg-css, html-css, …) rely on `input_source_loading` / `type_loading` early-return in `loadFileType`, same as today.

Do **not** collect import/QName names, destroy the parent tree, load deps, and reparse the parent. Do **not** scan regexp/`match`/`word/@name` text for `prefix:` to guess types to load — catalog type names collide with C++ `foo:` fragments and pull in extra languages.

Do not add process-global XML-load or zip-cache state. XML or zip/`jar:` InputSource changes must run `./tests/schemes/run.sh load` **and** `load packed`.

## File shape

```xml
<?xml version="1.0"?>
<hrc version="take5" xmlns="http://colorer.sf.net/2003/hrc">
  <!-- prototype | package | type -->
</hrc>
```

Typical: `proto.hrc` holds prototypes/packages; each type is one file. `annotation` (documentation / contributors / appinfo) is ignored by the parser.

`<hrc>` children:

| Element | Role |
|---------|------|
| `prototype` | Named language: detect + lazy `location` of the type |
| `package` | Same load path, not a user-facing file type (no filename/firstline) |
| `type` | Schemes, regions, entities, imports |

## Names

Local names unique per type. Cross-type: `type:name`. `<import type="def"/>` copies names; first import wins on clash. Unqualified lookup: current type, then imports in order.

Regions: CapitalCase (`StringQuote`). Types/packages: lowercase. Schemes: lowercase, dash/dot ok (`Comment.content`). Files: lowercase.

## Prototype

`name` = type name (NCName). `description` required in XSD. `group` optional. `targetNamespace` optional (XML languages).

| Child | Attrs | Meaning |
|-------|--------|---------|
| `location` | `link` URI | Type HRC; not loaded until the type is chosen |
| `filename` | text = `/RE/[ix]*`, `weight` default **2** | Match against file name |
| `firstline` | same RE, `weight` default **1** | Match against start of content (may span lines; `s` modifier allowed) |
| `parameters/param` | `name`, `value`, `description?` | Runtime profile; scheme `@if` / `@unless` |

Each matching filename/firstline adds its weight. Highest total wins; ties → first.

Later HRC with the same `name` **replaces** the whole prototype (unload). `hrcsettings.xml` only adds/updates params and, if any chooser is present, **replaces** the chooser list — [overrides.md](overrides.md).

## Package

Attrs: `name`, `description`, `group?`, `targetNamespace?`, `global?`. No filename/firstline. This loader: `global` empty or `"yes"` → load with prototypes; `"no"` → load with types.

## Type

`name` required. Children: `import`, `region`, `entity`, `scheme` (and annotation).

| Child | Attrs |
|-------|--------|
| `import` | `type` — import that type's names |
| `region` | `name`, `parent?` QName, `description?` |
| `entity` | `name`, `value` — macro in RE as `%name;` |
| `scheme` | `name`, `if?` / `unless?` param names |

`if` / `unless`: scheme body loaded only if the named param is / is not true; otherwise the scheme is empty. Params come from prototype `<parameters>` and can change via API.

Base scheme: `scheme/@name` == `type/@name`. Required for prototypes, not packages.

## Regions

A region is a named span. `parent` builds a tree; HRD walks parents if a name has no assign. Parse also builds a scheme enter/leave tree.

Convention `def:*` (package `def`, usually `default.hrc`): `Keyword`, `String`, `Number`, `Comment` / `LineComment`, `Symbol`, `Error`, `PairStart` / `PairEnd`, `Outlined`, `Text`. Pair regions must nest. `def:Error` → error list. `def:Outlined` → outliner. `def:empty` — stub scheme for a block that only paints start/end.

## Scheme children (match order)

Nodes are tried **in HRC order**. First match wins. After a match, position advances by the match (or `\m`/`\M` span).

| Element | Default `priority` |
|---------|-------------------|
| `keywords` | **low** |
| `regexp` | **normal** |
| `block` | **normal** |
| `inherit` | inlines another scheme's nodes here (no cycles) |

### keywords

`region` required (this loader skips the block if missing). `ignorecase` `yes`\|`no`. XSD default `yes`. **This loader:** `matchCase = (ignorecase != "yes")` — missing attr is **case-sensitive**. `worddiv`: char class (`[…]` or `%entity;`) for word edges; default = non-word chars.

- `word`: match only if both edges are word-dividers.
- `symb`: no edge check.
- Per-item `region` overrides the list.

### regexp

`match="/RE/[ix]*"` (also `region0`…`regionf` hex). Line-bound. Up to 16 captures.

| Attr | Meaning |
|------|---------|
| `region` | whole match (same as `region0` unless `\m`/`\M`) |
| `region0` | whole match / `\m`…`\M` |
| `region1`…`regionf` | capture 1…15 |
| named `(?{Name}…)` | region `Name` if it exists |

`%entity;` expands before compile. XML attr escaping: `&quot;`, `&lt;`, `&amp;`.

### block

Switch scheme while `start` matches until `end` matches. `scheme` QName **required**. Empty scheme: `def:empty`.

Start/end: attrs `start`/`end` or child `<start match="…"/>` / `<end match="…"/>` (CDATA for large RE).

| Attr | Meaning |
|------|---------|
| `region` | wrap (or inner span if `inner-region`) |
| `region0X` | start RE capture X (hex); `region00` = whole start |
| `region1X` | end RE capture X; `region10` = whole end |
| `priority` | start vs outer `end` (see below) |
| `content-priority` | if `low`, **all** nodes of the inner scheme act as `low` |
| `inner-region` | `yes`: wrap region is between start and end, not including them. `no` (default): wrap includes start and end |

`end` may use `\yN` / `\Y{name}` to copy start captures.

### inherit / virtual

```xml
<inherit scheme="other:scheme">
  <virtual scheme="inner" subst-scheme="replacement"/>
</inherit>
```

Inserts `other:scheme` nodes here. No cyclic inherit. Each `virtual` replaces inner scheme references `scheme` with `subst-scheme` (language embedding).

## Priority vs block end

Inner match beats outer `end` unless the inner node is `priority="low"`. Then outer `end` wins. Nested block: `priority` applies to that block's **start** vs outer `end`; an inner **end** always beats an outer **end**.

Low-priority regexp: `$` may match the parent block boundary (error highlighting).

## CRegExp

Must be `/.../` plus optional modifiers: `i` ignore case, `x` ignore spaces/newlines in the pattern, `s` `.` matches CR/LF — **only useful on `<firstline>`**; all other RE cannot cross a line.

Not Perl in lookaround: Colorer is `(foo)?=` / `(foo)?!` / `(foo)?#N` / `(foo)?~N`, not `(?=foo)`.

`?` after a quantifier → non-greedy (`*?` `+?` `??` `{n,m}?`).

### Atoms

| Token | Meaning |
|-------|---------|
| `^` `$` | line start / end |
| `.` | any except CR/LF |
| `[…]` `[^…]` | class; ranges `a-z`; unicode `[{Lu}]`; subtract `-[]`; intersect `\|[]` |
| `\xHH` `\x{HHHH}` | hex char |
| `\n` `\r` `\t` | LF / CR / tab |
| `\s` `\S` `\w` `\W` `\d` `\D` `\u` `\l` | space / word / digit / upper / lower (`\l` = not upper) |
| `\b` `\B` | word boundary / not |
| `\c` | previous char is non-word (Colorer) |
| `\#` | literal `#` if `#` not `a-z`/`1-9` |
| `( )` | capturing group |
| `(?{name} )` | named capture |
| `(?: )` / `(?{} )` | non-capturing |
| `\|` | alternation |
| `*` `+` `?` `{n}` `{n,}` `{n,m}` | quantifiers |
| `\N` | backref to group N; only if that group is literal (no operators) |

### Colorer-only (need `COLORERMODE`)

| Token | Meaning |
|-------|---------|
| `~` | start of parent scheme (after inner `start`) |
| `\m` `\M` | redefine match start / end (overlap / `region0` span) |
| `\yN` `\YN` `\y{name}` `\Y{name}` | in `end`: copy start group |

Lookaround operators (need a preceding atom/group): `?#N` look-behind N chars, `?~N` negative, `?=` look-ahead, `?!` negative look-ahead.

## catalog.xml

Loaded by `ParserFactory`. Current schemes: `xmlns="http://colorer.github.io/schema/v1/catalog"`.

```xml
<catalog>
  <hrc-sets>          <!-- optional log-location on this element (legacy) -->
    <location link="hrc/proto.hrc"/>
    <location link="hrc/auto"/>   <!-- directory: *.hrc only -->
  </hrc-sets>
  <hrd-sets>
    <hrd class="console|rgb|text" name="white" description="…">
      <location link="hrd/…"/>
    </hrd>
  </hrd-sets>
</catalog>
```

`link` relative to the catalog file, or `jar:common.zip!hrc/proto.hrc`. Directory `link`: first-level `*.hrc` only, skip `*.ent.hrc`. XML entities in catalog often alias `hrd` paths (including `jar:`). DTD `SYSTEM` with env vars: `env:$VAR/…` (not `jar:`) or `jar:$VAR/archive.zip!…` — [overrides.md](overrides.md).

Duplicate `prototype/@name` in a later HRC file **unloads** the first definition. `hrcsettings.xml` merges params / replaces choosers only; it cannot add types. Do not edit the base catalog to customize.

Colorer-schemes (separate repo; example sibling path `../Colorer-schemes`): `build.sh base` → `_build/base/` loose files; `build.sh base.packed` → `_build/base-packed/` zip + `jar:` links. XML/zip loader changes must test packed.

## HRD

Root `<hrd>`, then `<assign>`. Last assign for a name wins (files overlay).

| Attr | Meaning |
|------|---------|
| `name` | qualified region `type:Name` |
| `fore` `back` | `#` + 1–6 hex; omitted = transparent |
| `style` | digit (bold/italic/underline bits) |
| `stext` `etext` `sback` `eback` | text prefix/suffix mappings |

Missing assign → parent region in the HRC tree. Assigned but missing a channel → fill region of the active scheme at runtime. No ancestor properties → skip drawing (transparent). Default text: `def:Text`.
