#pragma once
#include "springjoint.h"
#include <unordered_map>
#include <vrm/gltf.h>
#include <vrm/humanpose.h>
#include <vrm/springbone.h>

namespace runtimescene {

struct RuntimeNode;
struct RuntimeMesh;
struct RuntimeSpringCollision;

using RenderFunc =
  std::function<void(const std::shared_ptr<libvrm::gltf::Mesh>&,
                     const RuntimeMesh&,
                     const float[16])>;

struct RuntimeScene
{
  std::shared_ptr<libvrm::gltf::GltfRoot> m_table;
  std::vector<std::shared_ptr<RuntimeNode>> m_nodes;
  std::vector<std::shared_ptr<RuntimeNode>> m_roots;

  std::optional<size_t> IndexOf(const std::shared_ptr<RuntimeNode>& node) const
  {
    return libvrm::gltf::_IndexOf<std::shared_ptr<RuntimeNode>>(m_nodes, node);
  }

  std::list<std::function<void(const RuntimeScene& scene)>> m_sceneUpdated;
  void RaiseSceneUpdated()
  {
    for (auto& callback : m_sceneUpdated) {
      callback(*this);
    }
  }

  libvrm::Time NextSpringDelta = libvrm::Time(0.0);
  std::shared_ptr<libvrm::gltf::GltfRoot> m_lastScene;

  std::unordered_map<std::shared_ptr<libvrm::gltf::Mesh>,
                     std::shared_ptr<RuntimeMesh>>
    m_meshMap;
  std::unordered_map<std::shared_ptr<libvrm::gltf::Node>,
                     std::shared_ptr<RuntimeNode>>
    m_nodeMap;

  std::unordered_map<std::shared_ptr<libvrm::vrm::SpringJoint>,
                     std::shared_ptr<RuntimeSpringJoint>>
    m_jointMap;
  std::unordered_map<std::shared_ptr<libvrm::vrm::SpringBone>,
                     std::shared_ptr<RuntimeSpringCollision>>
    m_springCollisionMap;

  RuntimeScene(const std::shared_ptr<libvrm::gltf::GltfRoot>& table);
  void Reset();

  std::shared_ptr<RuntimeMesh> GetRuntimeMesh(
    const std::shared_ptr<libvrm::gltf::Mesh>& mesh);
  std::shared_ptr<RuntimeNode> GetRuntimeNode(
    const std::shared_ptr<libvrm::gltf::Node>& node);

  std::shared_ptr<RuntimeSpringJoint> GetRuntimeJoint(
    const std::shared_ptr<libvrm::vrm::SpringJoint>& joint);
  std::shared_ptr<RuntimeSpringCollision> GetRuntimeSpringCollision(
    const std::shared_ptr<libvrm::vrm::SpringBone>& springBone);

  // void Render(const std::shared_ptr<libvrm::gltf::Scene>& scene,
  //             const RenderFunc& render,
  //             libvrm::IGizmoDrawer* gizmo);
  std::vector<libvrm::gltf::DrawItem> m_drawables;
  std::span<const libvrm::gltf::DrawItem> Drawables();

  std::vector<DirectX::XMFLOAT4X4> m_shapeMatrices;
  std::span<const DirectX::XMFLOAT4X4> ShapeMatrices();

  void SpringUpdate(const std::shared_ptr<libvrm::vrm::SpringBone>& solver,
                    libvrm::Time deltaForSimulation);
  void SpringDrawGizmo(const std::shared_ptr<libvrm::vrm::SpringBone>& solver,
                       libvrm::IGizmoDrawer* gizmo);
  void SpringColliderDrawGizmo(
    const std::shared_ptr<libvrm::vrm::SpringCollider>& collider,
    libvrm::IGizmoDrawer* gizmo);
  DirectX::XMVECTOR SpringColliderPosition(
    const std::shared_ptr<libvrm::vrm::SpringCollider>& collider);

  void NodeConstraintProcess(const libvrm::vrm::NodeConstraint& constraint,
                             const std::shared_ptr<RuntimeNode>& dst);

  // humanpose
  std::vector<libvrm::vrm::HumanBones> m_humanBoneMap;
  std::vector<DirectX::XMFLOAT4> m_rotations;
  libvrm::vrm::HumanPose m_pose;
  libvrm::vrm::HumanPose UpdateHumanPose();
  void SetHumanPose(const libvrm::vrm::HumanPose& pose);
  void SyncHierarchy();
  void DrawGizmo(libvrm::IGizmoDrawer* gizmo);
};

// void SetInitialPose()
// {
//   for (auto& node : m_nodes) {
//     node->Transform = node->InitialTransform;
//   }
//   for (auto& root : m_roots) {
//     root->CalcWorldMatrix(true);
//   }
//   RaiseSceneUpdated();
// }

}
