#pragma once
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace libvrm {
namespace fileutil {
template<typename T>
static std::vector<T>
ReadAllBytes(const std::filesystem::path& filename)
{
  std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
  if (!ifs) {
    return {};
  }
  auto pos = ifs.tellg();
  auto size = pos / sizeof(T);
  if (pos % sizeof(T)) {
    ++size;
  }
  std::vector<T> buffer(size);
  ifs.seekg(0, std::ios::beg);
  ifs.read((char*)buffer.data(), pos);
  return buffer;
}
}
}
