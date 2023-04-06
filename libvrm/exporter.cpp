#include "vrm/exporter.h"
#include "vrm/animation.h"
#include "vrm/glb.h"
#include "vrm/jsons.h"
#include "vrm/scene.h"

namespace gltf {

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

  ExportNodesScenes(scene);
  ExportMeshes(scene);
  ExportBuffersViewsAccessors(scene);
  ExportAnimations(scene);

  m_writer.object_close();
}

void
Exporter::ExportNodesScenes(const Scene& scene)
{
  if (scene.m_nodes.empty()) {
    return;
  }
  m_writer.key("nodes");
  {
    m_writer.array_open();
    for (auto& node : scene.m_nodes) {
      m_writer.object_open();
      if (auto mesh_index = node->Mesh) {
        m_writer.key("mesh");
        m_writer.value(*mesh_index);
      }
      m_writer.object_close();
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

void
Exporter::ExportMeshes(const Scene& scene)
{
  if (scene.m_meshes.empty()) {
    return;
  }
  m_writer.key("meshes");
  {
    m_writer.array_open();
    for (auto& mesh : scene.m_meshes) {
      m_writer.object_open();
      m_writer.key("primitives");
      m_writer.array_open();
      int index = 0;
      for (auto& prim : mesh->m_primitives) {
        m_writer.object_open();
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
      }
      m_writer.array_close();
      m_writer.object_close();
    }
    m_writer.array_close();
  }
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

void
Exporter::ExportAnimations(const Scene& scene)
{
  if (scene.m_animations.empty()) {
    return;
  }

  m_writer.key("animations");
  {
    m_writer.array_open();
    for (auto& animation : scene.m_animations) {
      m_writer.object_open();

      for (auto& [k, v] : animation->m_translationMap) {
        ExportAnimationTranslation(scene, k, v);
      }
      for (auto& [k, v] : animation->m_rotationMap) {
        ExportAnimationRotation(scene, k, v);
      }
      for (auto& [k, v] : animation->m_scaleMap) {
        ExportAnimationScale(scene, k, v);
      }
      for (auto& [k, v] : animation->m_weightsMap) {
        ExportAnimationWeights(scene, k, v);
      }

      m_writer.object_close();
    }
    m_writer.array_close();
  }
}

void
Exporter::ExportAnimationTranslation(const Scene& scene,
                                     uint32_t node,
                                     const Curve<DirectX::XMFLOAT3>& curve)
{
}

void
Exporter::ExportAnimationRotation(const Scene& scene,
                                  uint32_t node,
                                  const Curve<DirectX::XMFLOAT4>& curve)
{
}

void
Exporter::ExportAnimationScale(const Scene& scene,
                               uint32_t node,
                               const Curve<DirectX::XMFLOAT3>& curve)
{
}

void
Exporter::ExportAnimationWeights(const Scene& scene,
                                 uint32_t node,
                                 const WeightsCurve& curve)
{
}

}
