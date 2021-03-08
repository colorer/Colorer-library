#include <colorer/utils/Environment.h>

uUnicodeString Environment::getOSVariable(const char* name)
{
#ifdef _MSC_VER
  size_t sz = 0;
  getenv_s(&sz, nullptr, 0, name);
  if (sz == 0) {
    return nullptr;
  }
  std::vector<char> value(sz + 1);
  getenv_s(&sz, &value[0], sz, name);
  return std::make_unique<UnicodeString>(&value[0], int32_t(sz - 1));
#else
  const char* const value = std::getenv(name);
  return value ? std::make_unique<UnicodeString>(value) : nullptr;
#endif
}
