#include "vrm/scene.h"
#include "vrm/animation.h"
#include "vrm/bvh.h"
#include "vrm/dmath.h"
#include "vrm/glb.h"
#include "vrm/jsonpath.h"
#include "vrm/material.h"
#include "vrm/mesh.h"
#include "vrm/node.h"
#include "vrm/skin.h"
#include "vrm/springbone.h"
#include <DirectXMath.h>
#include <array>
#include <expected>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <optional>
#include <span>
#include <sstream>
#include <string>
#include <vector>

namespace libvrm::gltf {
Scene::Scene() {}

Scene::~Scene()
{
  std::cout << "Scene::~Scene()" << std::endl;
}

void
Scene::Traverse(const EnterFunc& enter,
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

void
Scene::TraverseJson(const EnterJson& enter,
                    const LeaveJson& leave,
                    nlohmann::json* item)
{
  if (!item) {
    // root
    m_jsonpath = "/";
    auto size = m_jsonpath.size();
    for (auto& kv : m_gltf.Json.items()) {
      m_jsonpath += kv.key();
      TraverseJson(enter, leave, &kv.value());
      m_jsonpath.resize(size);
    }
    return;
  }

  if (enter(*item, m_jsonpath)) {
    if (item->is_object()) {
      auto size = m_jsonpath.size();
      for (auto& kv : item->items()) {
        m_jsonpath.push_back(DELIMITER);
        m_jsonpath += kv.key();
        TraverseJson(enter, leave, &kv.value());
        m_jsonpath.resize(size);
      }
    } else if (item->is_array()) {
      auto size = m_jsonpath.size();
      for (int i = 0; i < item->size(); ++i) {
        std::stringstream ss;
        ss << i;
        auto str = ss.str();
        m_jsonpath.push_back(DELIMITER);
        m_jsonpath += str;
        TraverseJson(enter, leave, &(*item)[i]);
        m_jsonpath.resize(size);
      }
    }
    if (leave) {
      leave();
    }
  }
}

std::shared_ptr<gltf::Node>
Scene::GetBoneNode(vrm::HumanBones bone)
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
Scene::GetBoundingBox() const
{
  BoundingBox bb{};
  for (auto& node : m_nodes) {
    bb.Extend(node->WorldInitialTransform.Translation);
    bb.Extend(node->WorldInitialTransform.Translation);
    if (node->Mesh) {
      auto mesh_bb = node->Mesh->GetBoundingBox();
      bb.Extend(dmath::transform(mesh_bb.Min, node->WorldInitialMatrix()));
      bb.Extend(dmath::transform(mesh_bb.Max, node->WorldInitialMatrix()));
    }
  }
  return bb;
}

std::span<const DrawItem>
Scene::Drawables()
{
  m_drawables.clear();
  for (auto& node : m_nodes) {
    if (node->Mesh) {
      m_drawables.push_back({
        .Mesh = node->Mesh,
      });
      DirectX::XMStoreFloat4x4(&m_drawables.back().Matrix,
                               node->WorldInitialMatrix());
    }
  }
  return m_drawables;
}

}
