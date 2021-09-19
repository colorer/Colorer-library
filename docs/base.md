# Библиотека схем

Основной частью библиотеки colorer является библиотека схем.

**Библиотека схем** - это набор описаний схем типов файлов и стилей раскраски, используемых библиотекой colorer для работы с файлами.\
**Схема** - файлы с расширешнием hrc. Hrc это формат описания структуры типа файла, который разбирает colorer. В файле описывается как логически разделить его на отдельные регионы,
к которым потом будут применяться стили раскраски.\
**Стиль раскраски** - файлы с расширешнием hrd. Hrd используются для сопоставления регионов, полученных на выходе разборы структуры файла по hrc-схеме, цветам или заменяющей их
информации. Цвета могут быть заданы в любом формате - интерпретация зависит от целевой программы, использующей библиотеку.\
**catalog.xml** - центральный файл бибилиотеки схем. В нём описываются пути до hrc и hrd файлов.

## catalog.xml

Для старта работы в colorer необходимо передать путь до catalog.xml. Формат xml файла имеет фиксированную структуру, описанную
в [catalog.xsd](https://colorer.github.io/schema/v1/catalog.xsd). Пример:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE catalog [
        <!ENTITY catalog-console SYSTEM "hrd/catalog-console.xml">
        <!ENTITY catalog-rgb SYSTEM "hrd/catalog-rgb.xml">
        <!ENTITY catalog-text SYSTEM "hrd/catalog-text.xml">
        ]>
<catalog xmlns="http://colorer.github.io/schema/v1/catalog" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
         xsi:schemaLocation="http://colorer.github.io/schema/v1/catalog https://colorer.github.io/schema/v1/catalog.xsd">
    <hrc-sets>
        <location link="hrc/proto.hrc"/>
        <location link="hrc/auto"/>
    </hrc-sets>
    <hrd-sets>
        &catalog-console;
        &catalog-rgb;
        &catalog-text;
    </hrd-sets>
</catalog>
```

В библиотеке схем широко используется возможность синтаксиса xml по вставке одного файла в другой - entity. Запись вида

```
<!ENTITY catalog-console SYSTEM "hrd/catalog-console.xml">
```

в начале файла говорит о том, что вместо `&catalog-console;` в xml файле будет подставляться содержимое файла `hrd/catalog-console.xml`. В случае относительного пути, путь
смотрится от текущего файла.

В блоке `hrc-sets` задаются пути до hrc файлов. Это может быть путь как до конкретного файла, так и до папки. В случае указания пути до папки производится поиск файлов с
расширением `.hrc` только на первом уровне, во вложенные папки не заходит.

В блоке `hrd-sets` задаются пути до файлов цветовых стилей.

Если упростить структуру файла, убрав вставки внешних файлов, то catalog.xml выглядит так:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<catalog xmlns="http://colorer.github.io/schema/v1/catalog" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
         xsi:schemaLocation="http://colorer.github.io/schema/v1/catalog https://colorer.github.io/schema/v1/catalog.xsd">
    <hrc-sets>
        <location link="hrc/proto.hrc"/>
        <location link="hrc/auto"/>
    </hrc-sets>
    <hrd-sets>
        <hrd class="console" name="default" description="Aqua on blue">
            <location link="hrd/console/default.hrd"/>
        </hrd>
        <hrd class="rgb" name="default" description="White (crimsoned)">
            <location link="hrd/rgb/white.hrd"/>
        </hrd>
        <hrd class="text" name="tags" description="HTML italic, underline indention">
            <location link="hrd/text/tags.hrd"/>
        </hrd>
    </hrd-sets>
</catalog>
```

## Работа с путями до файлов

Ссылки на файлы/папки в библиотеке схем могут быть следующих типов:

1. **Полный путь**\
   Обычный полный путь до файла на файловой системе. Например `c:\user\test\1.xml`, `/home/user/1.xml`.
2. **Длинный путь**\
   В Windows поддерживаются длинные пути (до 32767 имволов). Полный путь может быть указан через `\\?\`, например `\\?\c:\user\test\1.xml`
3. **Относительный путь**\
   Если в пути нет привязки к диску (windows) или корню (linux), а так же встречаются ссылки на текущую папку `./` или на родительскую `../`, то такие пути считаются
   относительными. Такой путь рассматрвиается относительно файла, в котором он описан. Т.е. если путь `..\1.xml` указан в `link` атрибуте файла `c:\users\test\base\catalog.xml`, то
   обрабатываться будет файл `c:\users\test\1.xml`.
4. **Файл в архиве**
   Файлы библиотеки схем могут храниться в zip архиве. Это позволяет упростить доставку схем, когда их десятки/сотни. Путь, указывающий на файл в архиве имеет следующий
   формат `jar:путь_до_архива!путь_до_файла_в_архиве`. Например `jar:common.jar!base/cpp.hrc`. Приэтом путь до архива так же может быть полным/длинным/относительным.

Для всех указанных типов путей поддерживается использование переменных окружения в формате текущей операционной системы. Например, `%COLORER_HOME%\catalog\catalog.xml`.

Поддерживается работа с symlink на файлы. Работа идет с файлом и полным путем до него, на который ссылается symlink. Т.е. при обработке относительного пути отсчет идет от
оригинального файла. Например:

* библиотека схем находится в c:\user\test\base.
* создан symlink c:\catalog.xml на c:\user\test\base\catalog.xml
* при работе с симлинком поиск файла hrc/proto.hrc будет вестись по пути c:\user\test\base\hrc\proto.hrc