#include "vrm/exporter.h"
#include "vrm/glb.h"
#include "vrm/jsons.h"
#include "vrm/scene.h"

gltf::Exported
gltf::Exporter::Export(const Scene& scene)
{
  Exported exported;
  jsons::Writer writer([&exported](std::string_view str) {
    auto size = exported.JsonChunk.size();
    exported.JsonChunk.resize(size + str.size());
    std::copy((const uint8_t*)str.data(),
              (const uint8_t*)str.data() + str.size(),
              exported.JsonChunk.data() + size);
    ;
  });

  gltf::BinWriter binWriter([&exported](std::span<const uint8_t> values) {
    auto size = exported.BinChunk.size();
    exported.BinChunk.resize(size + values.size());
    std::copy(values.data(),
              values.data() + values.size(),
              exported.BinChunk.data() + size);
  });

  writer.object_open();
  writer.key("asset");
  {
    writer.object_open();
    writer.key("version");
    writer.value("2.0");
    writer.object_close();
  }
  writer.key("scene");
  {
    writer.value(0);
  }
  writer.key("scenes");
  {
    writer.array_open();
    writer.object_open();
    writer.key("nodes");
    writer.array_open();
    writer.value(0);
    writer.array_close();
    writer.object_close();
    writer.array_close();
  }
  writer.key("nodes");
  {
    writer.array_open();
    for (auto& node : scene.m_nodes) {
      writer.object_open();
      if (auto mesh_index = node->Mesh) {
        writer.key("mesh");
        writer.value(*mesh_index);
      }
      writer.object_close();
    }
    writer.array_close();
  }
  writer.key("meshes");
  {
    writer.array_open();
    for (auto& mesh : scene.m_meshes) {
      writer.object_open();
      writer.key("primitives");
      writer.array_open();
      int index = 0;
      for (auto& prim : mesh->m_primitives) {
        writer.object_open();
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
        //   auto indices_index = binWriter.PushAccessor<const uint8_t>(
        //     { indices.data(), indices.size() });
        //   writer.key("indices");
        //   writer.value(indices_index);
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
          auto indices_index = binWriter.PushAccessor<const uint16_t>(
            { indices.data(), indices.size() });
          writer.key("indices");
          writer.value(indices_index);

        } else {
          std::vector<uint32_t> indices;
          for (int i = 0; i < prim.drawCount; ++i, ++index) {
            auto vertex_index = mesh->m_indices[index];
            indices.push_back(vertex_index);

            auto v = mesh->m_vertices[vertex_index];
            positions.push_back(v.Position);
          }
          auto indices_index = binWriter.PushAccessor<const uint32_t>(
            { indices.data(), indices.size() });
          writer.key("indices");
          writer.value(indices_index);
        }

        auto position_accessor_index =
          binWriter.PushAccessor<const DirectX::XMFLOAT3>(
            { positions.data(), positions.size() });
        writer.key("attributes");
        writer.object_open();
        writer.key("POSITION");
        writer.value(position_accessor_index);
        writer.object_close();

        writer.object_close();
      }
      writer.array_close();
      writer.object_close();
    }
    writer.array_close();
  }
  writer.key("accessors");
  {
    writer.array_open();
    for (auto& accessor : binWriter.Accessors) {
      writer.object_open();
      writer.key("bufferView");
      writer.value(accessor.BufferView);
      writer.key("count");
      writer.value(accessor.Count);
      writer.key("byteOffset");
      writer.value(0);
      writer.key("type");
      writer.value(gltf::type_str(accessor.Type));
      writer.key("componentType");
      writer.value((int)accessor.ComponentType);
      writer.object_close();
    }
    writer.array_close();
  }
  writer.key("bufferViews");
  {
    writer.array_open();
    for (auto& bufferView : binWriter.BufferViews) {
      writer.object_open();
      writer.key("buffer");
      writer.value(0);
      writer.key("byteOffset");
      writer.value(bufferView.ByteOffset);
      writer.key("byteLength");
      writer.value(bufferView.ByteLength);
      writer.object_close();
    }
    writer.array_close();
  }
  writer.key("buffers");
  {
    writer.array_open();
    {
      writer.object_open();
      writer.object_close();
    }
    writer.array_close();
  }
  writer.object_close();

  return exported;
}
