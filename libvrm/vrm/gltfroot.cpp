#include "gltfroot.h"
#include "dmath.h"
#include "humanoid/humanskeleton.h"
#include "node.h"
#include "node_state.h"
#include "spring_bone.h"
#include <DirectXMath.h>
#include <array>
#include <expected>
#include <fstream>
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

std::span<NodeState>
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

} // namespace
