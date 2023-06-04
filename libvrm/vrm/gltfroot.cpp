#include "gltfroot.h"
#include "animation/spring_bone.h"
#include "base_mesh.h"
#include "dmath.h"
#include "gltf.h"
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

namespace libvrm::gltf {
GltfRoot::GltfRoot() {}

GltfRoot::~GltfRoot()
{
  std::cout << "Scene::~Scene()" << std::endl;
}

void
GltfRoot::Traverse(const EnterFunc& enter,
                   const LeaveFunc& leave,
                   const std::shared_ptr<gltf::Node>& node)
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

std::shared_ptr<gltf::Node>
GltfRoot::GetBoneNode(vrm::HumanBones bone)
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

runtimescene::BoundingBox
GltfRoot::GetBoundingBox() const
{
  runtimescene::BoundingBox bb{};
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
            runtimescene::BoundingBox mesh_bb = {
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
  }
  else{
    for(auto &node: m_nodes)
    {
        bb.Extend(node->WorldInitialTransform.Translation);
    }    
  }
  return bb;
}

std::span<const DrawItem>
GltfRoot::Drawables()
{
  m_drawables.clear();
  if (m_gltf) {
    for (uint32_t i = 0; i < m_nodes.size(); ++i) {
      auto node = m_nodes[i];
      auto gltfNode = m_gltf->Nodes[i];
      if (auto mesh = gltfNode.Mesh()) {
        m_drawables.push_back({
          .Mesh = *mesh,
        });
        DirectX::XMStoreFloat4x4(&m_drawables.back().Matrix,
                                 node->WorldInitialMatrix());
      }
    }
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

}
