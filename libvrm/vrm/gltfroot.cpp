#include <Windows.h>

#include "dmath.h"
#include "gltfroot.h"
#include "humanoid/humanskeleton.h"
#include "node.h"
#include "spring_bone.h"
#include <DirectXMath.h>
#include <GL/GL.h>
#include <array>
#include <boneskin/base_mesh.h>
#include <boneskin/node_state.h>
#include <expected>
#include <fstream>
#include <gltfjson/bin_writer.h>
#include <gltfjson/glb.h>
#include <iostream>
#include <limits>
#include <list>
#include <optional>
#include <span>
#include <sstream>
#include <string>
#include <vector>

namespace libvrm {

GltfRoot::GltfRoot() {}

GltfRoot::~GltfRoot() {}

std::tuple<std::shared_ptr<Node>, uint32_t>
GltfRoot::GetBoneNode(HumanBones bone)
{
  for (uint32_t i = 0; i < m_nodes.size(); ++i) {
    auto& node = m_nodes[i];
    if (auto humanoid = node->Humanoid) {
      if (*humanoid == bone) {
        return { node, i };
      }
    }
  }
  return {};
}

BoundingBox
GltfRoot::GetBoundingBox() const
{
  BoundingBox bb{};
  if (m_gltf) {
    for (uint32_t i = 0; i < m_gltf->Nodes.size(); ++i) {
      auto gltfNode = m_gltf->Nodes[i];
      auto node = m_nodes[i];
      bb.Extend(node->WorldInitialTransform.Translation);
      if (auto meshId = gltfNode.MeshId()) {
        auto mesh = m_gltf->Meshes[*meshId];
        for (auto prim : mesh.Primitives) {
          auto position_accessor_index = *prim.Attributes()->POSITION_Id();
          auto accessor = m_gltf->Accessors[position_accessor_index];

          auto& min = accessor.Min;
          auto& max = accessor.Max;
          if (min.size() == 3 && max.size() == 3) {
            BoundingBox mesh_bb = {
              { min[0], min[1], min[2] },
              { max[0], max[1], max[2] },
            };
            bb.Extend(
              dmath::transform(mesh_bb.Min, node->WorldInitialMatrix()));
            bb.Extend(
              dmath::transform(mesh_bb.Max, node->WorldInitialMatrix()));
          }
        }
      }
    }
  } else if (m_nodes.size()) {
    for (auto& node : m_nodes) {
      bb.Extend(node->WorldInitialTransform.Translation);
    }
  } else {
    return { { 0, 0, 0 }, { 0, 0, 0 } };
  }
  return bb;
}

void
GltfRoot::InitializeNodes()
{
  for (auto& root : m_roots) {
    root->CalcWorldInitialMatrix(true);
    root->CalcShape();
  }
}

std::span<boneskin::NodeState>
GltfRoot::NodeStates()
{
  if (m_gltf) {
    m_drawables.resize(m_nodes.size());
    for (uint32_t i = 0; i < m_nodes.size(); ++i) {
      auto node = m_nodes[i];
      auto gltfNode = m_gltf->Nodes[i];
      auto& item = m_drawables[i];
      item.MorphMap.clear();
      if (auto meshId = gltfNode.MeshId()) {
        auto mesh = m_gltf->Meshes[*meshId];
        for (int i = 0; i < mesh.Weights.size(); ++i) {
          item.MorphMap[i] = mesh.Weights[i];
        }
      }
      DirectX::XMStoreFloat4x4(&item.Matrix, node->WorldInitialMatrix());
    }
  } else {
    m_drawables.clear();
  }
  return m_drawables;
}

std::span<const DirectX::XMFLOAT4X4>
GltfRoot::ShapeMatrices()
{
  m_shapeMatrices.clear();
  for (auto& node : m_nodes) {
    m_shapeMatrices.push_back({});
    auto shape = DirectX::XMLoadFloat4x4(&node->ShapeMatrix);
    DirectX::XMStoreFloat4x4(&m_shapeMatrices.back(),
                             shape * node->WorldInitialMatrix());
  }
  return m_shapeMatrices;
}

std::shared_ptr<HumanSkeleton>
GltfRoot::GetHumanSkeleton()
{
  auto skeleton = std::make_shared<HumanSkeleton>();
  std::unordered_map<std::shared_ptr<Node>, uint32_t> indexMap;
  for (auto& node : m_nodes) {
    if (auto humanoid = node->Humanoid) {
      auto index = (uint32_t)skeleton->Bones.size();
      indexMap.insert({ node, index });
      skeleton->Bones.push_back({
        *humanoid,
        node->WorldInitialTransform.Translation,
        node->WorldInitialTransform.Rotation,
      });
      for (auto parent = node->Parent.lock(); parent;
           parent = parent->Parent.lock()) {
        auto found = indexMap.find(parent);
        if (found != indexMap.end()) {
          skeleton->Bones.back().ParentIndex = found->second;
          break;
        }
      }
    }
  }
  return skeleton;
}

void
GltfRoot::SelectNode(const std::shared_ptr<libvrm::Node>& node)
{
  m_selected = node;
}

bool
GltfRoot::IsSelected(const std::shared_ptr<libvrm::Node>& node) const
{
  return m_selected == node;
}

void
GltfRoot::InitializeGltf()
{
  auto json = std::make_shared<gltfjson::tree::Node>();
  json->Set(gltfjson::ObjectValue());
  json->SetProperty(u8"asset", gltfjson::ObjectValue());
  json->SetProperty(u8"images", gltfjson::ArrayValue());
  json->SetProperty(u8"textures", gltfjson::ArrayValue());
  json->SetProperty(u8"materials", gltfjson::ArrayValue());
  json->SetProperty(u8"buffers", gltfjson::ArrayValue());
  json->SetProperty(u8"bufferViews", gltfjson::ArrayValue());
  json->SetProperty(u8"accessors", gltfjson::ArrayValue());
  json->SetProperty(u8"meshes", gltfjson::ArrayValue());
  json->SetProperty(u8"nodes", gltfjson::ArrayValue());
  json->SetProperty(u8"skins", gltfjson::ArrayValue());
  json->SetProperty(u8"scenes", gltfjson::ArrayValue());
  m_gltf = std::make_shared<gltfjson::Root>(json);
}

uint32_t
GltfRoot::AddBufferView(uint32_t byteOffset,
                        uint32_t byteLength,
                        uint32_t byteStride)
{
  auto index = m_gltf->BufferViews.size();
  auto bufferView = m_gltf->BufferViews.m_json->Add(gltfjson::ObjectValue());
  bufferView->SetProperty(u8"byteOffset", (float)byteOffset);
  bufferView->SetProperty(u8"byteLength", (float)byteLength);
  if (byteStride) {
    bufferView->SetProperty(u8"byteStride", (float)byteStride);
  }
  return index;
}

uint32_t
GltfRoot::AddAccessor(uint32_t bufferViewIndex,
                      uint32_t accessorCount,
                      uint32_t accessorOffset,
                      uint32_t elementType,
                      const char* type)
{
  auto index = m_gltf->Accessors.size();
  auto accessor = m_gltf->Accessors.m_json->Add(gltfjson::ObjectValue());

  accessor->SetProperty(u8"bufferView", (float)bufferViewIndex);
  if (accessorOffset) {
    accessor->SetProperty(u8"byteOffset", (float)accessorOffset);
  }
  accessor->SetProperty(u8"componentType", (float)elementType);
  accessor->SetProperty(u8"type", std::u8string((const char8_t*)type));
  accessor->SetProperty(u8"count", (float)accessorCount);

  return index;
}

uint32_t
GltfRoot::AddMesh(const std::shared_ptr<boneskin::BaseMesh>& mesh)
{
  // glTF
  auto index = m_gltf->Meshes.size();
  auto meshJson = m_gltf->Meshes.m_json->Add(gltfjson::ObjectValue{});
  auto attributes =
    meshJson->SetProperty(u8"attributes", gltfjson::ObjectValue{});

  gltfjson::BinWriter w(m_bytes);

  {
    auto view =
      w.PushBufferView(mesh->m_vertices.data(), mesh->m_vertices.size());
    auto bufferView =
      AddBufferView(view.ByteOffset, view.ByteOffset, sizeof(boneskin::Vertex));
    {
      auto accessor =
        AddAccessor(bufferView, mesh->m_vertices.size(), 0, GL_FLOAT, "VEC3");
      attributes->SetProperty(u8"POSITION", (float)accessor);
    }
    {
      auto accessor =
        AddAccessor(bufferView, mesh->m_vertices.size(), 12, GL_FLOAT, "VEC3");
      attributes->SetProperty(u8"NORMAL", (float)accessor);
    }
    {
      auto accessor =
        AddAccessor(bufferView, mesh->m_vertices.size(), 24, GL_FLOAT, "VEC2");
      attributes->SetProperty(u8"TEXCOORD_0", (float)accessor);
    }
  }

  {
    auto view =
      w.PushBufferView(mesh->m_indices.data(), mesh->m_indices.size());
    auto bufferView =
      AddBufferView(view.ByteOffset, view.ByteOffset, sizeof(uint32_t));
    {
      auto accessor = AddAccessor(
        bufferView, mesh->m_indices.size(), 0, GL_UNSIGNED_INT, "SCALAR");
      meshJson->SetProperty(u8"indices", (float)accessor);
    }
  }

  m_bin.Bytes = m_bytes;

  return index;
}

std::shared_ptr<Node>
GltfRoot::CreateNode(const std::string& name)
{
  auto pNode = std::make_shared<libvrm::Node>(name);
  m_nodes.push_back(pNode);

  // glTF
  auto obj = m_gltf->Nodes.m_json->Add(gltfjson::ObjectValue{});
  obj->SetProperty(u8"name", std::u8string((const char8_t*)name.c_str()));
  return pNode;
}

} // namespace
