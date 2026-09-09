# Catalog overlays and settings (this library)

How apps customize a **base schemes catalog** without editing it. Human docs: [https://colorer.github.io](https://colorer.github.io) (`advanced/custom.md`, `advanced/hrcsettings.md`, `advanced/catalog-xml.md`, `advanced/file-paths.md`). Sibling checkout of that site: `../colorer.github.io`.

HRC grammar is [hrc-ref.md](hrc-ref.md). This note is load order, prototype replace vs merge, `hrcsettings.xml`, user HRC/HRD paths, env vars, and URI forms.

**Do not patch files inside Colorer-schemes `_build/` or a released catalog** to add types, params, or chooser REs. Overlay after `loadCatalog`. Editing `catalog.xml` / `proto.hrc` of the base library is lost on upgrade.

---

## Load order

Canonical sequence (`ParserFactory`; CLI `tools/colorer/ConsoleTools.cpp` does this):

1. `loadCatalog(catalog_path)` — parse `catalog.xml`, load every `<hrc-sets>/<location>` as prototypes, register `<hrd-sets>/<hrd>`.
2. `loadHrcPath(user_hrc)` — extra `.hrc` file or directory (prototypes only).
3. `loadHrcSettings(hrcsettings_path, user_defined)` — merge into **existing** prototypes.
4. `loadHrdPath(user_hrd)` — extra `.hrd` file or directory, or an `<hrd-sets>` index file.

`loadCatalog` / `loadHrcPath` / `loadHrcSettings` / `loadFileType` are exclusive per library. After a type is loaded, parsers may run concurrently.

Empty `loadCatalog(nullptr)` uses **`COLORER_CATALOG`**. If that is unset → `ParserFactoryException`. There is no built-in filesystem search for `catalog.xml`.

---

## Two overlay mechanisms (do not mix them up)

| Goal | Use | Effect |
|------|-----|--------|
| New language / replace whole prototype (`name`, `group`, `description`, `location`, choosers, params) | Later HRC via `loadHrcPath` (or catalog `auto/`) with the **same** `prototype/@name` | First prototype is **unloaded**; the new one is loaded. Log: `Duplicate prototype`. |
| Change **params** and/or **filename/firstline** of an already loaded type | `hrcsettings.xml` via `loadHrcSettings` | Params add/update. Choosers **replace the whole list** if any `<filename>`/`<firstline>` is present. |
| New or extra HRD style | `loadHrdPath` | New `HrdNode` appended. Lookup is **first** `class`+`name` match (`getHrdNode`). |
| Runtime param for this session only | `FileType::setParamValue` | Writes `user_value`; not persisted. `getParamValue` prefers `user_value` over default. |

`hrcsettings.xml` **cannot** create a prototype. Unknown `name` → warn and skip. **Cannot** change `group`, `description`, or `location` — those need a duplicate prototype in HRC.

A later HRC prototype with the same `name` is a **full replace**, not a merge. To keep the original scheme file, copy the whole prototype (including `<location link="…"/>` relative to **your** HRC file).

---

## `hrcsettings.xml`

Root `<hrc-settings>`, children `<prototype name="…">` only. Code: `HrcLibrary::Impl::updatePrototype`.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<hrc-settings>
  <prototype name="cpp">
    <filename weight="100">/\.cxx$/</filename>
    <parameters>
      <param name="maxlinelength" value="8000" description="…"/>
      <param name="my-app-flag" value="true" description="app-only"/>
    </parameters>
  </prototype>
</hrc-settings>
```

### Params (`<param>`)

- Nested in `<parameters>` (preferred, same as HRC) **or** direct child of `<prototype>` (compat; HRC files themselves do **not** accept a bare `<param>` under `<prototype>` — only `<parameters>`).
- Missing param → **add** (`addParam`).
- Existing param → update **default** `value` (`setParamDefaultValue`) and `description` if given. Does not set `user_value`.
- Does **not** wipe other params.

`scheme/@if` / `@unless` read `getParamValue` (user override, else default). Apps that persist UI checkboxes should save them themselves (or write `hrcsettings.xml`); the library does not write this file.

### Choosers (`<filename>`, `<firstline>`)

If the settings prototype has **at least one** of these, `chooserVector` is **cleared** then rebuilt from the settings file only. To *add* a pattern, copy the original choosers from the base prototype and append yours.

Weights: same as HRC (`filename` default **2**, `firstline` default **1**).

### `loadHrcSettings(location, user_defined)`

| Call | Empty / null `location` |
|------|-------------------------|
| `user_defined == true` | Use **`COLORER_HRC_SETTINGS`**. If unset, skip (not an error). |
| `user_defined == false` | No-op (reserved for a fixed app-level path; none is compiled in). |

Non-empty `location` always loads that file (normalized path).

CLI: `-cs<path>` (`user_defined=true`). Env `COLORER_HRC_SETTINGS` if `-cs` omitted.

---

## User HRC (`loadHrcPath`)

File → `loadProtoTypes`. Directory → non-recursive `*.hrc`, skip `*.ent.hrc` (entity fragments).

Prototype may live in its own HRC (no `<location>` → type body is this file) or in a `proto.hrc`-style index with `<location link="…"/>`.

Same-name prototype after catalog: unload + replace (see above). New names: extra file types.

Less preferred (still in base catalog): directory `hrc/auto` listed last in `catalog.xml`. Do not edit distro `empty.hrc`. Prefer an app user path over writing into the catalog tree.

CLI: `-cu<path>`.

---

## User HRD (`loadHrdPath`)

Filesystem only (no `jar:` branch).

| Path | Expected root | Behavior |
|------|----------------|----------|
| Directory | each `*.hrd` | Root must be `<hrd class="" name="" description="">` with `<assign>` children. Registered as a new style. |
| File | `<hrd-sets>` | Same shape as catalog `<hrd-sets>`: `<hrd class name description><location link="…"/></hrd>`. |

`addHrd` **appends**. `getHrdNode(class, name)` returns the **first** node with that pair. A user file that reuses an existing `class`+`name` does **not** replace the catalog style. Use a **new `name`**, or overlay assigns by listing extra `<location>` on the **same** catalog `<hrd>` (last assign for a region wins inside `StyledHRDMapper` / `TextHRDMapper`).

Default mapper class: `rgb` (`createStyledMapper(nullptr, …)`). Default style name: **`COLORER_HRD`**, else `"default"`. Text mapper class is always `text`.

CLI: `-cd<path>` (user HRD), `-i<name>` (style name).

---

## Environment (library defaults)

Used when the corresponding API/CLI argument is omitted:

| Variable | Role |
|----------|------|
| `COLORER_CATALOG` | Path to `catalog.xml` (`loadCatalog(nullptr)`). Required if no path given. |
| `COLORER_HRC_SETTINGS` | Path to `hrcsettings.xml` when `loadHrcSettings(nullptr, true)`. Optional. |
| `COLORER_HRD` | HRD style **name** (not a file path) when mapper `nameID` is null. |

---

## Path / URI forms

Apply to `catalog.xml` `location/@link`, HRC `location/@link`, HRD locations, and XML `SYSTEM` entities. Relative links resolve against the **parent resource** (the file that contains the link). Missing scheme → `file://`.

| Form | Example | Notes |
|------|---------|--------|
| Absolute | `/home/u/catalog.xml`, `c:\u\catalog.xml` | Windows long paths: `\\?\C:\…` |
| Relative | `hrc/proto.hrc`, `../foo.hrc` | Against the current XML/HRC/HRD file |
| Zip | `jar:schemes.zip!hrc/proto.hrc` | Needs `COLORER_USE_ZIPINPUTSOURCE`. Archive path may be relative or use env vars |
| Env in ordinary paths | Windows `%VAR%`; Unix `$VAR` / `${VAR}` | `Environment::expandEnvironment` on normalized paths |
| Env in DTD `SYSTEM` | `env:$FARHOME/hrd/x.xml` | Prefix **`env:` required** if the path is not `jar:`. Always `$VAR` (no `{}`, any OS). `jar:$FARHOME/a.zip!…` does **not** use `env:` |

Catalog directory `location`: first-level `*.hrc` only, skip `*.ent.hrc`. Catalog HRD `SYSTEM` entities (Colorer-schemes `Hard` catalog) stay on `xmlMyExternalEntityLoader`.

Packed catalogs rewrite HRC `link` to `jar:…`. XML or zip InputSource changes: `./tests/schemes/run.sh load` **and** `load packed`.

---

## `catalog.xml` (index only)

`xmlns="http://colorer.github.io/schema/v1/catalog"`. XSD: https://colorer.github.io/schema/v1/catalog.xsd.

```xml
<catalog>
  <hrc-sets>
    <location link="hrc/proto.hrc"/>
    <location link="hrc/auto"/>
  </hrc-sets>
  <hrd-sets>
    <hrd class="console|rgb|text" name="default" description="…">
      <location link="hrd/…"/>
    </hrd>
  </hrd-sets>
</catalog>
```

HRC locations load **in document order**. Put user/auto after `proto.hrc` so duplicate names replace the base prototype.

Scheme library flavors (Colorer-schemes): unpacked / packed (`jar:` HRC) / allpacked. Functionally equivalent if zip is enabled. See [colorer-schemes-tests](../colorer-schemes-tests/SKILL.md).

---

## CLI (`colorer` tools)

```
-c<path>    catalog.xml
-cs<path>   hrcsettings.xml
-cu<path>   user HRC file or directory
-cd<path>   user HRD file or directory
-i<name>    HRD style name
```
