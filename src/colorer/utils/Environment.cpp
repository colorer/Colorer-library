#include <colorer/utils/Environment.h>

uUnicodeString Environment::getOSVariable(const char* name)
{
#ifdef _MSC_VER
  spdlog::debug("get system environment {0}", name);
  size_t sz = 0;
  getenv_s(&sz, nullptr, 0, name);
  if (sz == 0) {
    spdlog::debug("{0} not set", name);
    return nullptr;
  }
  std::vector<char> value(sz + 1);
  getenv_s(&sz, &value[0], sz, name);
  auto result = std::make_unique<UnicodeString>(&value[0], int32_t(sz - 1));
  spdlog::debug("{0} = '{1}'", name, *result);
  return result;
#else
  spdlog::debug("get system environment {0}", name);
  const char* const value = std::getenv(name);
  if (!value) {
    spdlog::debug("{0} not set", name);
    return nullptr;
  } else {
    spdlog::debug("{0} = '{1}'", name, value);
    return std::make_unique<UnicodeString>(value);
  }
#endif
}
