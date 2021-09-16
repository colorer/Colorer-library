#include <colorer/common/UStr.h>
#include <colorer/utils/Environment.h>
#include <filesystem>
#ifdef WIN32
#include <windows.h>
#else
#include <regex>
#endif

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

uUnicodeString Environment::expandEnvironment(const UnicodeString* path)
{
#ifdef WIN32
  std::wstring path_ws = UStr::to_stdwstr(path);
  size_t i = ExpandEnvironmentStringsW(path_ws.c_str(), nullptr, 0);
  auto temp = std::make_unique<wchar_t[]>(i);
  ExpandEnvironmentStringsW(path_ws.c_str(), temp.get(), static_cast<DWORD>(i));
  return std::make_unique<UnicodeString>(temp.get());
#else
  std::string text = UStr::to_stdstr(path);
  static const std::regex env_re {R"--(\$\{([^}]+)\})--"};
  std::smatch match;
  while (std::regex_search(text, match, env_re)) {
    auto const from = match[0];
    auto const var_name = match[1].str().c_str();
    text.replace(from.first, from.second, std::getenv(var_name));
  }
  return std::make_unique<UnicodeString>(text.c_str());
#endif
}

uUnicodeString Environment::normalizePath(const UnicodeString* path)
{
  auto expanded_string = Environment::expandEnvironment(path);
  auto fpath = std::filesystem::path(UStr::to_stdstr(expanded_string));
  fpath = fpath.lexically_normal();
  if (std::filesystem::is_symlink(fpath)) {
    fpath = std::filesystem::read_symlink(fpath);
  }
  return std::make_unique<UnicodeString>(fpath.c_str());
}
