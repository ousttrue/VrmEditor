#pragma once
#include "scene.h"
#include <span>
#include <stdint.h>
#include <vector>

namespace gltf {
struct Scene;

struct Exporter
{
  std::vector<uint8_t> JsonChunk;
  std::vector<uint8_t> BinChunk;
  void Export(const Scene& scene);
};
}
