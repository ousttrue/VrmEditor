#pragma once
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace libvrm {

static std::vector<uint8_t>
ReadAllBytes(const std::filesystem::path& filename)
{
  std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
  if (!ifs) {
    return {};
  }
  auto pos = ifs.tellg();
  std::vector<uint8_t> buffer(pos);
  ifs.seekg(0, std::ios::beg);
  ifs.read((char*)buffer.data(), pos);
  return buffer;
}

} // namespace
