#include "fs_util.h"
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

