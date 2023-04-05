#pragma once
#include "scene.h"
#include <span>
#include <stdint.h>
#include <vector>

namespace gltf {
struct Scene;

struct Exported
{
  std::vector<uint8_t> JsonChunk;
  std::vector<uint8_t> BinChunk;
};

struct Exporter
{
  std::vector<uint8_t> Buffer;
  Exported Export(const Scene& scene);
};
}
