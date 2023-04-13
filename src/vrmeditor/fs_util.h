#pragma once
#include <filesystem>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#include <shlobj.h>
#include <shlwapi.h>

inline std::filesystem::path
get_home()
{
  char buf[1024];
  GetEnvironmentVariableA("USERPROFILE", buf, sizeof(buf));
  return buf;
}

inline void
shell_open(const std::filesystem::path& path)
{
  auto str = path.wstring();
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

#else
#include <cstdio>
#include <stdlib.h>

inline std::filesystem::path
get_home()
{
  return std::getenv("HOME");
}

inline void
shell_open(const std::filesystem::path& path)
{
  std::stringstream ss;
  ss << "xdg-open " << path;
  system(ss.str().c_str());
}

#endif
