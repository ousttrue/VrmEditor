#pragma once
#include <expected>
#include <functional>
#include <optional>
#include <span>
#include <stdint.h>
#include <string>
#include <tuple>

namespace gltf {
struct Glb
{
  std::span<const uint8_t> json;
  std::span<const uint8_t> bin;
  static std::optional<Glb> parse(std::span<const uint8_t> bytes);
};

}
