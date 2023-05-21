#include "gltfroot.h"
#include "animation/spring_bone.h"
#include "base_mesh.h"
#include "dmath.h"
#include "gltf.h"
#include "jsonpath.h"
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

// void
// Scene::TraverseJson(const EnterJson& enter,
//                     const LeaveJson& leave,
//                     nlohmann::json* item)
// {
//   if (!item) {
//     // root
//     m_jsonpath = "/";
//     auto size = m_jsonpath.size();
//     // for (auto& kv : m_gltf.Json.items()) {
//     //   m_jsonpath += kv.key();
//     //   TraverseJson(enter, leave, &kv.value());
//     //   m_jsonpath.resize(size);
//     // }
//     return;
//   }
//
//   if (enter(*item, m_jsonpath)) {
//     if (item->is_object()) {
//       auto size = m_jsonpath.size();
//       for (auto& kv : item->items()) {
//         m_jsonpath.push_back(DELIMITER);
//         m_jsonpath += kv.key();
//         TraverseJson(enter, leave, &kv.value());
//         m_jsonpath.resize(size);
//       }
//     } else if (item->is_array()) {
//       auto size = m_jsonpath.size();
//       for (int i = 0; i < item->size(); ++i) {
//         std::stringstream ss;
//         ss << i;
//         auto str = ss.str();
//         m_jsonpath.push_back(DELIMITER);
//         m_jsonpath += str;
//         TraverseJson(enter, leave, &(*item)[i]);
//         m_jsonpath.resize(size);
//       }
//     }
//     if (leave) {
//       leave();
//     }
//   }
// }

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
  for (auto& node : m_nodes) {
    bb.Extend(node->WorldInitialTransform.Translation);
    bb.Extend(node->WorldInitialTransform.Translation);
    // if (node->Mesh) {
    // auto mesh_bb = node->Mesh->GetBoundingBox();
    // bb.Extend(dmath::transform(mesh_bb.Min, node->WorldInitialMatrix()));
    // bb.Extend(dmath::transform(mesh_bb.Max, node->WorldInitialMatrix()));
    // }
  }
  return bb;
}

std::span<const DrawItem>
GltfRoot::Drawables()
{
  m_drawables.clear();
  for (uint32_t i = 0; i < m_nodes.size(); ++i) {
    auto node = m_nodes[i];
    auto& gltfNode = m_gltf.Nodes[i];
    if (gltfNode.Mesh) {
      m_drawables.push_back({
        .Mesh = *gltfNode.Mesh,
      });
      DirectX::XMStoreFloat4x4(&m_drawables.back().Matrix,
                               node->WorldInitialMatrix());
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
