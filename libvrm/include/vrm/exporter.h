#pragma once
#include "animation.h"
#include "jsons.h"
#include "scene.h"
#include <gltfjson/bin_writer.h>
#include <span>
#include <stdint.h>
#include <vector>

namespace libvrm {
namespace gltf {

struct Scene;
struct Primitive;

struct Exporter
{
  jsons::Writer m_writer;
  gltfjson::format::BinWriter m_binWriter;
  std::vector<uint8_t> JsonChunk;
  std::vector<uint8_t> BinChunk;

  Exporter();

  void Export(const Scene& scene);
  void ExportAsset(const Scene& scene);
  void ExportImage(const Scene& scene, const std::shared_ptr<Image>& image);
  void ExportTextureSampler(
    const Scene& scene,
    const std::shared_ptr<gltfjson::format::Sampler>& sampler);
  void ExportTexture(const Scene& scene,
                     const std::shared_ptr<Texture>& texture);
  void ExportMaterial(const Scene& scene,
                      const std::shared_ptr<Material>& material);
  void ExportMesh(const Scene& scene, const std::shared_ptr<Mesh>& mesh);
  uint32_t ExportMeshPrimitive(const Scene& scene,
                               const std::shared_ptr<Mesh>& mesh,
                               const Primitive& primitive,
                               uint32_t index);
  void ExportSkin(const Scene& scene, const std::shared_ptr<Skin>& skin);
  void ExportNode(const Scene& scene, const std::shared_ptr<Node>& node);

  void ExportBuffersViewsAccessors(const Scene& scene);
  void ExportAnimations(const Scene& scene);
};

}
}
