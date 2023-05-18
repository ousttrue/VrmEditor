#pragma once
#include "animation.h"
#include "jsons.h"
#include "gltfroot.h"
#include <gltfjson/bin_writer.h>
#include <span>
#include <stdint.h>
#include <vector>

namespace libvrm {
namespace gltf {

struct GltfRoot;
struct Primitive;

struct Exporter
{
  jsons::Writer m_writer;
  gltfjson::format::BinWriter m_binWriter;
  std::vector<uint8_t> JsonChunk;
  std::vector<uint8_t> BinChunk;

  Exporter();

  void Export(const GltfRoot& scene);
  void ExportAsset(const GltfRoot& scene);
  void ExportImage(const GltfRoot& scene, const std::shared_ptr<Image>& image);
  void ExportTextureSampler(
    const GltfRoot& scene,
    const std::shared_ptr<gltfjson::format::Sampler>& sampler);
  void ExportTexture(const GltfRoot& scene,
                     const std::shared_ptr<Texture>& texture);
  void ExportMaterial(const GltfRoot& scene,
                      const std::shared_ptr<Material>& material);
  void ExportMesh(const GltfRoot& scene, const std::shared_ptr<Mesh>& mesh);
  uint32_t ExportMeshPrimitive(const GltfRoot& scene,
                               const std::shared_ptr<Mesh>& mesh,
                               const Primitive& primitive,
                               uint32_t index);
  void ExportSkin(const GltfRoot& scene, const std::shared_ptr<Skin>& skin);
  void ExportNode(const GltfRoot& scene, const std::shared_ptr<Node>& node);

  void ExportBuffersViewsAccessors(const GltfRoot& scene);
  void ExportAnimations(const GltfRoot& scene);
};

}
}
