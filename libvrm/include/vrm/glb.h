#pragma once
#include <expected>
#include <filesystem>
#include <functional>
#include <optional>
#include <span>
#include <stdint.h>
#include <string>
#include <tuple>

namespace libvrm::gltf {
struct Glb
{
  std::span<const uint8_t> Json;
  uint32_t JsonPadding() const
  {
    auto json_padding = 0;
    if (Json.size() % 4) {
      json_padding = 4 - (Json.size() % 4);
    }
    return json_padding;
  }

  std::span<const uint8_t> Bin;
  uint32_t BinPadding() const
  {
    auto bin_padding = 0;
    if (Bin.size() % 4) {
      bin_padding = 4 - (Bin.size() % 4);
    }
    return bin_padding;
  }

  static std::optional<Glb> Parse(std::span<const uint8_t> bytes);
  uint32_t CalcSize() const
  {
    return
      // glb header
      12
      // json chunk
      + 8 + Json.size() +
      JsonPadding()
      // bin chunk
      + 8 + Bin.size() + BinPadding();
  }

  bool WriteTo(const std::filesystem::path& path);
};

}
