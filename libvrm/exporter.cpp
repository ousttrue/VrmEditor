#include "vrm/exporter.h"
#include "vrm/animation.h"
#include "vrm/glb.h"
#include "vrm/jsons.h"
#include "vrm/material.h"
#include "vrm/mesh.h"
#include "vrm/scene.h"

namespace libvrm::gltf {

Exporter::Exporter()
  : m_writer([this](std::string_view str) {
    auto size = JsonChunk.size();
    JsonChunk.resize(size + str.size());
    std::copy((const uint8_t*)str.data(),
              (const uint8_t*)str.data() + str.size(),
              JsonChunk.data() + size);
    ;
  })
  , m_binWriter([this](std::span<const uint8_t> values) {
    auto size = BinChunk.size();
    BinChunk.resize(size + values.size());
    std::copy(
      values.data(), values.data() + values.size(), BinChunk.data() + size);
  })
{
}

void
Exporter::Export(const Scene& scene)
{
  m_writer.object_open();

  m_writer.key("asset");
  {
    m_writer.object_open();
    m_writer.key("version");
    m_writer.value("2.0");
    m_writer.object_close();
  }

  // images
  if (scene.m_images.size()) {
    m_writer.key("images");
    {
      m_writer.array_open();
      for (auto& image : scene.m_images) {
        ExportImage(scene, image);
      }
      m_writer.array_close();
    }
  }

  // samplers
  if (scene.m_samplers.size()) {
    m_writer.key("samplers");
    {
      m_writer.array_open();
      for (auto& sampler : scene.m_samplers) {
        ExportTextureSampler(scene, sampler);
      }
      m_writer.array_close();
    }
  }

  // textures
  if (scene.m_textures.size()) {
    m_writer.key("textures");
    {
      m_writer.array_open();
      for (auto& texture : scene.m_textures) {
        ExportTexture(scene, texture);
      }
      m_writer.array_close();
    }
  }

  // materials
  if (scene.m_materials.size()) {
    m_writer.key("materials");
    {
      m_writer.array_open();
      for (auto& material : scene.m_materials) {
        ExportMaterial(scene, material);
      }
      m_writer.array_close();
    }
  }

  // meshes
  if (scene.m_meshes.size()) {
    // meshes
    m_writer.key("meshes");
    {
      m_writer.array_open();
      for (auto& mesh : scene.m_meshes) {
        ExportMesh(scene, mesh);
      }
      m_writer.array_close();
    }
  }

  // skins

  // nodes
  if (scene.m_nodes.size()) {
    m_writer.key("nodes");
    {
      m_writer.array_open();
      for (auto& node : scene.m_nodes) {
        ExportNode(node);
      }
      m_writer.array_close();
    }
    m_writer.key("scenes");
    {
      m_writer.array_open();
      m_writer.object_open();
      m_writer.key("nodes");
      m_writer.array_open();
      m_writer.value(0);
      m_writer.array_close();
      m_writer.object_close();
      m_writer.array_close();
    }
    m_writer.key("scene");
    {
      m_writer.value(0);
    }
  }

  // update bin
  ExportAnimations(scene);

  // last
  ExportBuffersViewsAccessors(scene);

  if (scene.m_vrma) {
    m_writer.key("extensions");
    m_writer.object_open();
    {
      m_writer.key("VRMC_vrm_animation");
      m_writer.object_open();
      {
        m_writer.key("expressions");
        m_writer.object_open();
        {
          m_writer.key("preset");
          m_writer.object_open();
          {
            m_writer.key("aa");
            m_writer.object_open();
            {
              m_writer.key("node");
              m_writer.value(0);
            }
            m_writer.object_close();
          }
          m_writer.object_close();
        }
        m_writer.object_close();
      }
      m_writer.object_close();
    }
    m_writer.object_close();
  }

  m_writer.object_close();
}

void
Exporter::ExportNode(const std::shared_ptr<Node>& node)
{
  m_writer.object_open();
  if (auto mesh_index = node->Mesh) {
    m_writer.key("mesh");
    m_writer.value(*mesh_index);
  }
  m_writer.object_close();
}

void
Exporter::ExportMesh(const Scene& scene, const std::shared_ptr<Mesh>& mesh)
{
  m_writer.object_open();
  if (mesh->Name.size()) {
    m_writer.key("name");
    m_writer.value(mesh->Name);
  }
  m_writer.key("primitives");
  m_writer.array_open();
  uint32_t index = 0;
  for (auto& prim : mesh->m_primitives) {
    index = ExportMeshPrimitive(scene, mesh, prim, index);
  }
  m_writer.array_close();
  m_writer.object_close();
}

uint32_t
Exporter::ExportMeshPrimitive(const Scene& scene,
                              const std::shared_ptr<Mesh>& mesh,
                              const Primitive& prim,
                              uint32_t index)
{
  m_writer.object_open();

  if (prim.material) {
    auto found = m_materialIndexMap.find(prim.material);
    if (found != m_materialIndexMap.end()) {
      m_writer.key("material");
      m_writer.value(found->second);
    }
  }

  std::vector<DirectX::XMFLOAT3> positions;

  // if (mesh->m_vertices.size() <= 255) {
  //   std::vector<uint8_t> indices;
  //   for (int i = 0; i < prim.drawCount; ++i, ++index) {
  //     auto vertex_index = mesh->m_indices[index];
  //     indices.push_back(vertex_index);
  //
  //     auto v = mesh->m_vertices[vertex_index];
  //     positions.push_back(v.Position);
  //   }
  //   auto indices_index = m_binWriter.PushAccessor<const uint8_t>(
  //     { indices.data(), indices.size() });
  //   m_writer.key("indices");
  //   m_writer.value(indices_index);
  //
  // } else
  if (mesh->m_vertices.size() <= 65535) {
    std::vector<uint16_t> indices;
    for (int i = 0; i < prim.drawCount; ++i, ++index) {
      auto vertex_index = mesh->m_indices[index];
      indices.push_back(vertex_index);

      auto v = mesh->m_vertices[vertex_index];
      positions.push_back(v.Position);
    }
    auto indices_index = m_binWriter.PushAccessor<const uint16_t>(
      { indices.data(), indices.size() });
    m_writer.key("indices");
    m_writer.value(indices_index);

  } else {
    std::vector<uint32_t> indices;
    for (int i = 0; i < prim.drawCount; ++i, ++index) {
      auto vertex_index = mesh->m_indices[index];
      indices.push_back(vertex_index);

      auto v = mesh->m_vertices[vertex_index];
      positions.push_back(v.Position);
    }
    auto indices_index = m_binWriter.PushAccessor<const uint32_t>(
      { indices.data(), indices.size() });
    m_writer.key("indices");
    m_writer.value(indices_index);
  }

  auto position_accessor_index =
    m_binWriter.PushAccessor<const DirectX::XMFLOAT3>(
      { positions.data(), positions.size() });
  m_writer.key("attributes");
  m_writer.object_open();
  m_writer.key("POSITION");
  m_writer.value(position_accessor_index);
  m_writer.object_close();

  m_writer.object_close();

  return index;
}

// {
//     "bufferView": 14,
//     "mimeType": "image/jpeg"
// }
void
Exporter::ExportImage(const Scene& scene, const std::shared_ptr<Image>& image)
{
  m_writer.object_open();
  m_writer.key("name");
  m_writer.value(image->Name);
  if (auto encoded = image->Encoded) {
    switch (encoded->Type) {
      case ImageType::Jpeg:
        m_writer.key("mimeType");
        m_writer.value("image/jpeg");
        break;
      case ImageType::Png:
        m_writer.key("mimeType");
        m_writer.value("image/png");
        break;
      default:
        assert(false);
        break;
    }
    auto index = m_binWriter.PushBufferView(encoded->Bytes);
    m_writer.key("bufferView");
    m_writer.value(index);
  } else {
    // encode to png ?
    assert(false);
  }
  m_writer.object_close();
}

void
Exporter::ExportTextureSampler(const Scene& scene,
                               const std::shared_ptr<TextureSampler>& sampler)
{
  m_writer.object_open();
  m_writer.object_close();
}

void
Exporter::ExportTexture(const Scene& scene,
                        const std::shared_ptr<Texture>& texture)
{
  m_writer.object_open();
  m_writer.key("source");
  m_writer.value(m_imageIndexMap[texture->Source]);
  m_writer.key("sampler");
  m_writer.value(m_samplerIndexMap[texture->Sampler]);
  m_writer.object_close();
}

void
Exporter::ExportMaterial(const Scene& scene,
                         const std::shared_ptr<Material>& material)
{
  m_writer.object_open();
  m_writer.key("name");
  m_writer.value(material->Name);
  m_writer.object_close();
}

void
Exporter::ExportBuffersViewsAccessors(const Scene& scene)
{
  if (m_binWriter.Accessors.size()) {
    m_writer.key("accessors");
    {
      m_writer.array_open();
      for (auto& accessor : m_binWriter.Accessors) {
        m_writer.object_open();
        m_writer.key("bufferView");
        m_writer.value(accessor.BufferView);
        m_writer.key("count");
        m_writer.value(accessor.Count);
        m_writer.key("byteOffset");
        m_writer.value(0);
        m_writer.key("type");
        m_writer.value(gltf::type_str(accessor.Type));
        m_writer.key("componentType");
        m_writer.value((int)accessor.ComponentType);
        m_writer.object_close();
      }
      m_writer.array_close();
    }
  }
  if (m_binWriter.BufferViews.size()) {
    m_writer.key("bufferViews");
    {
      m_writer.array_open();
      for (auto& bufferView : m_binWriter.BufferViews) {
        m_writer.object_open();
        m_writer.key("buffer");
        m_writer.value(0); // for glb
        m_writer.key("byteOffset");
        m_writer.value(bufferView.ByteOffset);
        m_writer.key("byteLength");
        m_writer.value(bufferView.ByteLength);
        m_writer.object_close();
      }
      m_writer.array_close();
    }

    m_writer.key("buffers");
    {
      m_writer.array_open();
      {
        m_writer.object_open();
        m_writer.object_close();
      }
      m_writer.array_close();
    }
  }
}

struct AnimationSampler
{
  uint32_t Input;
  uint32_t Output;
  AnimationInterpolationModes Interpolation;
};

struct AnimationChannel
{
  uint32_t Sampler;
  uint32_t Node;
  AnimationTargets Target;
};

struct AnimationExporter
{
  BinWriter& m_binWriter;
  std::vector<AnimationSampler> AnimationSamplers;
  std::vector<AnimationChannel> AnimationChannels;

  AnimationExporter(BinWriter& binWriter)
    : m_binWriter(binWriter)
  {
  }

  template<typename T>
  uint32_t ExportAnimationSampler(
    std::span<const float> times,
    std::span<const T> values,
    AnimationInterpolationModes mode = AnimationInterpolationModes::LINEAR)
  {
    auto index = AnimationSamplers.size();
    auto input_index =
      m_binWriter.PushAccessor<const float>({ times.data(), times.size() });
    auto output_index =
      m_binWriter.PushAccessor<const T>({ values.data(), values.size() });

    AnimationSamplers.push_back({
      .Input = input_index,
      .Output = output_index,
      .Interpolation = mode,
    });

    return index;
  }

  void ExportAnimationTranslation(uint32_t node,
                                  const Curve<DirectX::XMFLOAT3>& curve)
  {
    auto sampler =
      ExportAnimationSampler<DirectX::XMFLOAT3>(curve.Times, curve.Values);
    ExportAnimationChannel(sampler, node, AnimationTargets::TRANSLATION);
  }

  void ExportAnimationRotation(uint32_t node,
                               const Curve<DirectX::XMFLOAT4>& curve)
  {
    auto sampler =
      ExportAnimationSampler<DirectX::XMFLOAT4>(curve.Times, curve.Values);
    ExportAnimationChannel(sampler, node, AnimationTargets::ROTATION);
  }

  void ExportAnimationScale(uint32_t node,
                            const Curve<DirectX::XMFLOAT3>& curve)
  {
    auto sampler =
      ExportAnimationSampler<DirectX::XMFLOAT3>(curve.Times, curve.Values);
    ExportAnimationChannel(sampler, node, AnimationTargets::SCALE);
  }

  void ExportAnimationWeights(uint32_t node, const WeightsCurve& curve)
  {
    // auto sampler =
    //   ExportAnimationSampler<DirectX::XMFLOAT3>(curve.Times, curve.Values);
    // ExportAnimationChannel(sampler, node, AnimationTargets::SCALE);
  }

  void ExportAnimationChannel(uint32_t sampler,
                              uint32_t node,
                              AnimationTargets target)
  {
    AnimationChannels.push_back({
      .Sampler = sampler,
      .Node = node,
      .Target = target,
    });
  }
};

void
Exporter::ExportAnimations(const Scene& scene)
{
  if (scene.m_animations.empty()) {
    return;
  }

  m_writer.key("animations");
  m_writer.array_open();

  // perpare
  for (auto& animation : scene.m_animations) {
    AnimationExporter animationExporter(m_binWriter);

    for (auto& [k, v] : animation->m_translationMap) {
      animationExporter.ExportAnimationTranslation(k, v);
    }
    for (auto& [k, v] : animation->m_rotationMap) {
      animationExporter.ExportAnimationRotation(k, v);
    }
    for (auto& [k, v] : animation->m_scaleMap) {
      animationExporter.ExportAnimationScale(k, v);
    }
    for (auto& [k, v] : animation->m_weightsMap) {
      animationExporter.ExportAnimationWeights(k, v);
    }

    m_writer.object_open();
    {
      m_writer.key("name");
      m_writer.value(animation->m_name);

      m_writer.key("samplers");
      m_writer.array_open();
      for (auto& sampler : animationExporter.AnimationSamplers) {
        m_writer.object_open();
        m_writer.key("input");
        m_writer.value(sampler.Input);
        m_writer.key("output");
        m_writer.value(sampler.Output);
        m_writer.key("interpolation");
        m_writer.value(
          AnimationInterpolationModeNames[(int)sampler.Interpolation]);
        m_writer.object_close();
      }
      m_writer.array_close();

      m_writer.key("channels");
      m_writer.array_open();
      for (auto& channel : animationExporter.AnimationChannels) {
        m_writer.object_open();
        m_writer.key("sampler");
        m_writer.value(channel.Sampler);
        m_writer.key("target");
        m_writer.object_open();
        m_writer.key("node");
        m_writer.value(channel.Node);
        m_writer.key("path");
        m_writer.value(AnimationTargetNames[(int)channel.Target]);
        m_writer.object_close();
        m_writer.object_close();
      }
      m_writer.array_close();
    }
    m_writer.object_close();
  }

  m_writer.array_close();
}
}
