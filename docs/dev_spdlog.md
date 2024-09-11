# Использование spdlog

Если вы решили использовать spdlog для логирования в вашем приложении, то для вывода строк с типом
UnicodeString вам потребуются formatter для этого типа.

## ICU UnicodeString
```cpp
#include "fmt/format.h"
#include "unicode/unistr.h"

namespace fmt {
template <>
struct formatter<icu::UnicodeString>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const icu::UnicodeString& p, FormatContext& ctx)
  {
    std::string result8;
    p.toUTF8String(result8);
    return format_to(ctx.out(), "%", result8);
  }
};

}  // namespace fmt
```

## Colorer strings (legacy) -  UnicodeString

```cpp
#include "fmt/format.h"
#include <string>
#include "colorer/strings/legacy/UnicodeString.h"

namespace fmt {
template <>
struct formatter<UnicodeString>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const UnicodeString& p, FormatContext& ctx)
  {
    std::string result8=p.getChars();
    return format_to(ctx.out(), "%", result8);
  }
};

}  // namespace fmt
```