#include "fs_util.h"
#include <Windows.h>
#include <shlobj.h>
#include <shlwapi.h>

std::filesystem::path
get_home()
{
  char buf[1024];
  GetEnvironmentVariableA("USERPROFILE", buf, sizeof(buf));
  return buf;
}

void
shell_open(const std::filesystem::path& path)
{
  auto absolute = std::filesystem::absolute(path);
  auto str = absolute.wstring();
  SHELLEXECUTEINFOW info{
    .cbSize = sizeof(SHELLEXECUTEINFOW),
    .fMask = NULL,
    .hwnd = NULL,
    .lpVerb = NULL,
    .lpFile = str.c_str(),
    .lpParameters = NULL,
    .lpDirectory = NULL,
    .nShow = SW_MAXIMIZE,
    .hInstApp = NULL,
  };
  ShellExecuteExW(&info);
}

std::filesystem::path
get_exe()
{
  char szModulePath[MAX_PATH];
  GetModuleFileNameA(NULL, szModulePath, std::size(szModulePath));
  return szModulePath;
}

std::string
get_env(const std::string& name)
{
  char buf[32767];
  GetEnvironmentVariableA(name.c_str(), buf, std::size(buf));
  return buf;
}
