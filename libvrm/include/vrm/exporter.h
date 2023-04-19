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
using TextureSamplerIndexMap =
  std::unordered_map<std::shared_ptr<TextureSampler>, size_t>;
using TextureIndexMap = std::unordered_map<std::shared_ptr<Texture>, size_t>;
using MaterialIndexMap = std::unordered_map<std::shared_ptr<Material>, size_t>;
using MeshIndexMap = std::unordered_map<std::shared_ptr<Mesh>, size_t>;
using SkinIndexMap = std::unordered_map<std::shared_ptr<Skin>, size_t>;

struct Exporter
{
  jsons::Writer m_writer;
  BinWriter m_binWriter;
  std::vector<uint8_t> JsonChunk;
  std::vector<uint8_t> BinChunk;

  ImageIndexMap m_imageIndexMap;
  TextureSamplerIndexMap m_samplerIndexMap;
  TextureIndexMap m_textureIndexMap;
  MaterialIndexMap m_materialIndexMap;
  MeshIndexMap m_meshIndexMap;
  SkinIndexMap m_skinIndexMap;

  Exporter();

  void Export(const Scene& scene);
  void ExportImage(const Scene& scene, const std::shared_ptr<Image>& image);
  void ExportTextureSampler(const Scene& scene,
                            const std::shared_ptr<TextureSampler>& sampler);
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
  void ExportNode(const std::shared_ptr<Node>& node);

  void ExportBuffersViewsAccessors(const Scene& scene);
  void ExportAnimations(const Scene& scene);
};

}
}
