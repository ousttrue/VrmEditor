#include "gltfroot.h"
#include "base_mesh.h"
#include "dmath.h"
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

GltfRoot::~GltfRoot()
{
  std::cout << "Scene::~Scene()" << std::endl;
}

void
GltfRoot::Traverse(const EnterFunc& enter,
                   const LeaveFunc& leave,
                   const std::shared_ptr<Node>& node)
{
  if (node) {
    if (enter(node)) {
      for (auto& child : node->Children) {
        Traverse(enter, leave, child);
      }
      if (leave) {
        leave();
      }
    }
  } else {
    // root
    for (auto& child : m_roots) {
      Traverse(enter, leave, child);
    }
  }
}

std::shared_ptr<Node>
GltfRoot::GetBoneNode(HumanBones bone)
{
  for (auto& node : m_nodes) {
    if (auto humanoid = node->Humanoid) {
      if (*humanoid == bone) {
        return node;
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
      if (gltfNode.Mesh()) {
        auto mesh = m_gltf->Meshes[*gltfNode.Mesh()];
        for (auto prim : mesh.Primitives) {
          auto position_accessor_index = *prim.Attributes()->POSITION();
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
  } else {
    for (auto& node : m_nodes) {
      bb.Extend(node->WorldInitialTransform.Translation);
    }
  }
  return bb;
}

void
GltfRoot::UpdateDrawables(
  std::unordered_map<uint32_t, std::shared_ptr<DrawItem>>& nodeDrawMap)
{
  if (m_gltf) {
    for (uint32_t i = 0; i < m_nodes.size(); ++i) {
      auto node = m_nodes[i];
      auto gltfNode = m_gltf->Nodes[i];

      if (auto mesh = gltfNode.Mesh()) {
        auto found = nodeDrawMap.find(i);
        std::shared_ptr<DrawItem> item;
        if (found != nodeDrawMap.end()) {
          item = found->second;
        } else {
          item = std::make_shared<DrawItem>();
          nodeDrawMap.insert({ i, item });
        }

        item->Mesh = *mesh;
        DirectX::XMStoreFloat4x4(&item->Matrix, node->WorldInitialMatrix());
        item->MorphMap.clear();
        item->SkinningMatrices = {};
      }
    }
  }
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

} // namespace
