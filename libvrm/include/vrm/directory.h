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

const std::string BASE64_PREFIX[]{
    "data:application/octet-stream;base64,",
    "data:application/gltf-buffer;base64,",
};

struct Directory {
  std::filesystem::path Base;
  std::unordered_map<std::filesystem::path, std::vector<uint8_t>> FileCaches;

  std::expected<std::span<const uint8_t>, std::string>
  GetBuffer(std::string_view uri) {
    auto found = FileCaches.find(uri);
    if (found != FileCaches.end()) {
      // use cache
      return found->second;
    }

    if (uri.starts_with("data:")) {
      // return std::unexpected{"base64 not implemented"};
      for (auto &prefix : BASE64_PREFIX) {
        if (uri.starts_with(prefix)) {
          auto decoded = gltf::Decode(uri.substr(std::size(prefix)));
          FileCaches.insert({uri, decoded});
          return FileCaches[uri];
        }
      }
      return std::unexpected{"not implemented base64"};
    }

    auto path = Base / uri;
    if (auto bytes = ReadAllBytes(path)) {
      FileCaches.insert({uri, *bytes});
      return FileCaches[uri];
    } else {
      return bytes;
    }
  }
};
