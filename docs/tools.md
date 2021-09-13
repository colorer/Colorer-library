# Tools

## Colorer

Конольная программа для работы с возможностями библиотеки.

```
Usage: colorer (command) (parameters) (logging parameters) [<filename>]
 Commands:
  -l         Lists all available languages
  -lt        Lists all available languages (HRC types)
  -ll        Lists and loads full HRC database
  -r         RE tests
  -h         Generates plain coloring from <filename> (uses 'rgb' hrd class)
  -ht        Generates plain coloring from <filename> using tokens output
  -v         Runs viewer on file <fname> (uses 'console' hrd class)
  -p<n>      Runs parser in profile mode (if <n> specified, makes <n> loops)
  -f         Forwards input file into output with specified encodings
 Parameters:
  -c<path>   Uses specified 'catalog.xml' file
  -i<name>   Loads specified hrd rules from catalog
  -t<type>   Tries to use type <type> instead of type autodetection
  -ls<name>  Use file <name> as input linking data source for href generation
  -o<name>   Use file <name> as output stream
  -ln        Add line numbers into the colorized file
  -db        Disable BOM start symbol output in Unicode encodings
  -dc        Disable information header in generator's output
  -ds        Disable HTML symbol substitutions in generator's output
  -dh        Disable HTML header and footer output
 Logging parameters (default logging off):
  -eh<name>  Log file name prefix
  -ed<name>  Log file directory
  -el<name>  Log level (off, debug, info, warning, error). Default value is off.
```

### Значения по умолчанию

В поведении библиотеки заложено следующее поведение по умолчанию, которое применимо и для программ, основанных на ней:

* Если не задан путь до catalog.xml, то производится поиск в переменной окружения **COLORER_CATALOG**.
* Класс стиля раскраски при преобразовании в реальные цвета (Style) по умолчанию "rgb".
* Класс стиля раскраски при преобразовании в текстовое представление (Text) по умолчанию "text".
* Если не задано имя стиля раскраски, то проверяется переменная окружения **COLORER_HRD**. Если и там пусто, то используется "default" стиль.

### Переменные окружения

* **COLORER_CATALOG** - путь до catalog.xml. Обрабатывается только в случае если не задан путь через параметры.
* **COLORER_HRD** - имя стиля раскраски. Обрабатывается только в случае если не задан через параметры.