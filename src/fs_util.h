#pragma once
#include <cstdio>
#include <filesystem>

inline std::filesystem::path
get_home()
{
#ifdef _WIN32
  return std::getnenv("USERPROFILE");
#else
  return std::getenv("HOME");
#endif
}
