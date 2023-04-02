#pragma once
#include <filesystem>

#ifdef _WIN32
#include <Windows.h>

inline std::filesystem::path
get_home()
{
  char buf[1024];
  GetEnvironmentVariableA("USERPROFILE", buf, sizeof(buf));
  return buf;
}

#else
#include <cstdio>

inline std::filesystem::path
get_home()
{
  return std::getenv("HOME");
}
#endif
