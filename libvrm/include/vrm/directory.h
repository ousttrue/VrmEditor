#pragma once
#include <expected>
#include <filesystem>
#include <fstream>
#include <span>
#include <stdint.h>
#include <unordered_map>
#include <vector>

inline std::expected<std::vector<uint8_t>, std::string>
ReadAllBytes(const std::filesystem::path &path) {
  std::ifstream ifs(path, std::ios::binary | std::ios::ate);
  if (!ifs) {
    return std::unexpected{"fail to open"};
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

  std::expected<std::span<const uint8_t>, std::string>
  GetBuffer(std::string_view relative) {
    auto path = Base / relative;
    auto found = FileCaches.find(path);
    if (found != FileCaches.end()) {
      // use cache
      return found->second;
    }

    if (auto bytes = ReadAllBytes(path)) {
      FileCaches.insert({path, *bytes});
      return FileCaches[path];
    } else {
      return bytes;
    }
  }
};
