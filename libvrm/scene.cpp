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
Scene::Render(const RenderFunc& render, IGizmoDrawer* gizmo)
{
  // update order
  // 1. ヒューマノイドボーンを解決
  // 2. 頭の位置が決まるのでLookAtを解決
  //  * Bone型 => leftEye, rightEye ボーンを回転
  //  * Expression型 => 次項
  // 3. ExpressionUpdate
  //  * Expression を Apply する
  // 4. コンストレイントを解決
  // 5. SpringBoneを解決

  // constraint
  for (auto& node : m_nodes) {
    if (auto constraint = node->Constraint) {
      constraint->Process(node);
    }
  }
  for (auto& root : m_roots) {
    root->CalcWorldMatrix(true);
  }

  // springbone
  for (auto& spring : m_springSolvers) {
    spring->Update(m_nextSpringDelta);
  }
  m_nextSpringDelta = {};

  if (m_expressions) {
    // VRM0 expression to morphTarget
    auto nodeToIndex = [nodes = m_nodes, expressions = m_expressions](
                         const std::shared_ptr<Node>& node) {
      for (uint32_t i = 0; i < nodes.size(); ++i) {
        if (node == nodes[i]) {
          return i;
        }
      }
      return (uint32_t)-1;
    };
    for (auto& [k, v] : m_expressions->EvalMorphTargetMap(nodeToIndex)) {
      auto& morph_node = m_nodes[k.NodeIndex];
      morph_node->MeshInstance->weights[k.MorphIndex] = v;
    }
  }

  // skinning
  for (auto& node : m_nodes) {
    if (auto mesh_index = node->Mesh) {
      auto mesh = m_meshes[*mesh_index];

      // mesh animation
      std::span<DirectX::XMFLOAT4X4> skinningMatrices;

      // skinning
      if (auto skin = node->Skin) {
        skin->CurrentMatrices.resize(skin->BindMatrices.size());

        auto rootInverse = DirectX::XMMatrixIdentity();
        if (auto root_index = skin->Root) {
          rootInverse = DirectX::XMMatrixInverse(nullptr, node->WorldMatrix());
        }

        for (int i = 0; i < skin->Joints.size(); ++i) {
          auto node = m_nodes[skin->Joints[i]];
          auto m = skin->BindMatrices[i];
          DirectX::XMStoreFloat4x4(&skin->CurrentMatrices[i],
                                   DirectX::XMLoadFloat4x4(&m) *
                                     node->WorldMatrix() * rootInverse);
        }

        skinningMatrices = skin->CurrentMatrices;
      }

      // apply morphtarget & skinning
      node->MeshInstance->applyMorphTargetAndSkinning(*mesh, skinningMatrices);
    }
  }
  DirectX::XMFLOAT4X4 m;
  for (auto& node : m_nodes) {
    if (auto mesh_index = node->Mesh) {
      auto mesh = m_meshes[*mesh_index];
      DirectX::XMStoreFloat4x4(&m, node->WorldMatrix());
      render(mesh, *node->MeshInstance, &m._11);
    }
  }

  for (auto& spring : m_springSolvers) {
    spring->DrawGizmo(gizmo);
  }
  for (auto& collider : m_springColliders) {
    collider->DrawGizmo(gizmo);
  }
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
