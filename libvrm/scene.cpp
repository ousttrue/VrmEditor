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

vrm::HumanPose
Scene::UpdateHumanPose()
{
  auto mult4 = [](const DirectX::XMVECTOR& q0,
                  const DirectX::XMVECTOR& q1,
                  const DirectX::XMVECTOR& q2,
                  const DirectX::XMVECTOR& q3) -> DirectX::XMVECTOR {
    return DirectX::XMQuaternionMultiply(
      DirectX::XMQuaternionMultiply(DirectX::XMQuaternionMultiply(q0, q1), q2),
      q3);
  };

  // retarget human pose
  m_humanBoneMap.clear();
  m_rotations.clear();
  for (auto& node : m_nodes) {
    if (auto humanoid = node->Humanoid) {
      m_humanBoneMap.push_back(humanoid->HumanBone);
      if (m_humanBoneMap.back() == vrm::HumanBones::hips) {
        // delta move
        DirectX::XMStoreFloat3(
          &m_pose.RootPosition,
          DirectX::XMVectorSubtract(
            DirectX::XMLoadFloat3(&node->WorldTransform.Translation),
            DirectX::XMLoadFloat3(&node->WorldInitialTransform.Translation)));
      }

      // retarget
      auto normalized =
        mult4(DirectX::XMQuaternionInverse(
                DirectX::XMLoadFloat4(&node->WorldInitialTransform.Rotation)),
              DirectX::XMLoadFloat4(&node->Transform.Rotation),
              DirectX::XMQuaternionInverse(
                DirectX::XMLoadFloat4(&node->InitialTransform.Rotation)),
              DirectX::XMLoadFloat4(&node->WorldInitialTransform.Rotation));

      m_rotations.push_back({});
      DirectX::XMStoreFloat4(&m_rotations.back(), normalized);
    }
  }
  m_pose.Bones = m_humanBoneMap;
  m_pose.Rotations = m_rotations;
  return m_pose;
}

void
Scene::SetHumanPose(const vrm::HumanPose& pose)
{
  assert(pose.Bones.size() == pose.Rotations.size());

  for (int i = 0; i < pose.Bones.size(); ++i) {
    if (auto node = GetBoneNode(pose.Bones[i])) {
      if (i == 0) {
        DirectX::XMStoreFloat3(
          &node->Transform.Translation,
          DirectX::XMVectorAdd(
            DirectX::XMLoadFloat3(&node->InitialTransform.Translation),
            DirectX::XMLoadFloat3(&pose.RootPosition)));
      }

      auto worldInitial =
        DirectX::XMLoadFloat4(&node->WorldInitialTransform.Rotation);
      auto q = DirectX::XMLoadFloat4(&pose.Rotations[i]);
      auto worldInitialInv = DirectX::XMQuaternionInverse(worldInitial);
      auto localInitial =
        DirectX::XMLoadFloat4(&node->InitialTransform.Rotation);

      // # retarget
      // normalized local rotation to unormalized hierarchy.
      DirectX::XMStoreFloat4(
        &node->Transform.Rotation,
        DirectX::XMQuaternionMultiply(
          DirectX::XMQuaternionMultiply(
            DirectX::XMQuaternionMultiply(worldInitial, q), worldInitialInv),
          localInitial));
    }
  }

  SyncHierarchy();
}

void
Scene::SyncHierarchy()
{
  // calc world
  auto enter = [](const std::shared_ptr<gltf::Node>& node) {
    node->CalcWorldMatrix();
    return true;
  };
  Traverse(enter, {});
}

std::shared_ptr<gltf::Node>
Scene::GetBoneNode(vrm::HumanBones bone)
{
  for (auto& node : m_nodes) {
    if (auto humanoid = node->Humanoid) {
      if (humanoid->HumanBone == bone) {
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
    bb.Extend(node->WorldTransform.Translation);
    bb.Extend(node->WorldTransform.Translation);
    if (auto mesh_index = node->Mesh) {
      auto mesh_bb = m_meshes[*mesh_index]->GetBoundingBox();
      bb.Extend(dmath::transform(mesh_bb.Min, node->WorldMatrix()));
      bb.Extend(dmath::transform(mesh_bb.Max, node->WorldMatrix()));
    }
  }
  return bb;
}

}
