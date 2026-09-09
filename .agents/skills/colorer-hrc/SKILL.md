---
name: colorer-hrc
description: HRC/HRD/catalog language and Colorer CRegExp semantics as this library implements them. Use when editing the parser, scheme nodes, CRegExp, catalog/HRD/XML load, regions, inherit/virtual, HRC fixtures under tests/unit/data, or overlay/customization (hrcsettings.xml, loadHrcPath/loadHrdPath, duplicate prototypes, COLORER_CATALOG / COLORER_HRC_SETTINGS / COLORER_HRD). Includes XML-load constraints: no SAX, recursive loadFileType on import/QName.
---

# HRC language (this library)

Full grammar and match semantics: [hrc-ref.md](hrc-ref.md). Read it before changing match order, region numbering, inherit/virtual, CRegExp, catalog/`jar:` load, or HRC fixtures.

Catalog overlays, `hrcsettings.xml`, user HRC/HRD paths, env defaults, URI/`env:`/`jar:` forms: [overrides.md](overrides.md). Read it before changing `ParserFactory` load order, prototype replace vs merge, or settings files.

Runtime coloring hot path (`CRegExp` filters, `TextParser` scheme search, `BaseEditor` window/invalidation): [core-parse.md](core-parse.md).

XML load constraints (no SAX, recursive `loadFileType` on import/QName) are in [hrc-ref.md](hrc-ref.md#xml-load-this-library).

Human site (customization, catalog, paths, CLI): https://colorer.github.io — sources often sit at `../colorer.github.io`.

Not for authoring new language types — that is Colorer-schemes skill `colorer-hrc-authoring` (`speed.md` there for scheme-side cost; the schemes repo may sit next to this one, e.g. `../Colorer-schemes`). Human HTML (legacy): https://colorer.sourceforge.net/hrc-ref/
