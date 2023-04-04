#pragma once
#include "scene.h"
#include <span>
#include <stdint.h>
#include <vector>

namespace gltf {
struct Scene;
struct Exporter
{
  std::vector<uint8_t> Buffer;
  std::span<const uint8_t> Export(const Scene& scene);
};
}
