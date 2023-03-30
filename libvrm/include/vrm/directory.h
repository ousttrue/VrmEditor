#pragma once
#include <filesystem>
#include <fstream>
#include <span>
#include <stdint.h>
#include <unordered_map>
#include <vector>

inline std::vector<uint8_t> ReadAllBytes(const std::filesystem::path &path) {
  std::ifstream ifs(path, std::ios::binary | std::ios::ate);
  if (!ifs) {
    return {};
  }
  auto pos = ifs.tellg();
  std::vector<uint8_t> buffer(pos);
  ifs.seekg(0, std::ios::beg);
  ifs.read((char *)buffer.data(), pos);
  return buffer;
}

struct Directory {
  std::filesystem::path Base;
  std::unordered_map<std::filesystem::path, std::vector<uint8_t>> FileCaches;

  std::span<const uint8_t> GetBuffer(std::string_view relative) {
    auto path = Base / relative;
    auto found = FileCaches.find(path);
    if (found != FileCaches.end()) {
      return found->second;
    }

    auto bytes = ReadAllBytes(path);
    FileCaches.insert({path, bytes});
    return FileCaches[path];
  }
};
