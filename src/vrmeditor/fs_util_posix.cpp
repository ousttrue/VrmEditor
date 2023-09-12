#include "fs_util.h"
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>

std::filesystem::path
get_home()
{
  return std::getenv("HOME");
}

void
shell_open(const std::filesystem::path& path)
{
  std::stringstream ss;
  ss << "xdg-open " << path;
  system(ss.str().c_str());
}

std::filesystem::path
get_exe()
{
  char buf[1024];
  auto readsize = readlink("/proc/self/exe", buf, std::size(buf));
  return std::string_view{ buf, buf + readsize };
}

std::string
get_env(const std::string& name)
{
  if (auto p = getenv(name.c_str())) {
    return p;
  }
  return {};
}
