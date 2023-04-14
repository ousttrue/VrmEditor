#pragma once
#include "animation.h"
#include "jsons.h"
#include "scene.h"
#include <span>
#include <stdint.h>
#include <vector>

namespace libvrm::gltf {

struct Scene;

struct Exporter
{
  jsons::Writer m_writer;
  BinWriter m_binWriter;
  std::vector<uint8_t> JsonChunk;
  std::vector<uint8_t> BinChunk;

  Exporter();

  void Export(const Scene& scene);
  void ExportMeshes(const Scene& scene);
  void ExportNodesScenes(const Scene& scene);
  void ExportBuffersViewsAccessors(const Scene& scene);
  void ExportAnimations(const Scene& scene);
};

}
