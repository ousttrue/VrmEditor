#pragma once
#include "animation.h"
#include "jsons.h"
#include "scene.h"
#include <span>
#include <stdint.h>
#include <vector>

namespace libvrm {
namespace gltf {

struct Scene;
struct Primitive;

using ImageIndexMap = std::unordered_map<std::shared_ptr<Image>, size_t>;
using MaterialIndexMap = std::unordered_map<std::shared_ptr<Material>, size_t>;

struct Exporter
{
  jsons::Writer m_writer;
  BinWriter m_binWriter;
  std::vector<uint8_t> JsonChunk;
  std::vector<uint8_t> BinChunk;

  ImageIndexMap m_imageIndexMap;
  MaterialIndexMap m_materialIndexMap;

  Exporter();

  void Export(const Scene& scene);
  void ExportMesh(const Scene& scene, const std::shared_ptr<Mesh>& mesh);
  uint32_t ExportMeshPrimitive(const Scene& scene,
                               const std::shared_ptr<Mesh>& mesh,
                               const Primitive& primitive,
                               uint32_t index);
  void ExportMaterial(const Scene& scene,
                      const std::shared_ptr<Material>& material);

  void ExportNodesScenes(const Scene& scene);
  void ExportNode(const std::shared_ptr<Node>& node);

  void ExportBuffersViewsAccessors(const Scene& scene);
  void ExportAnimations(const Scene& scene);
};

}
}
