#pragma once
#include "springjoint.h"
#include <unordered_map>
#include <vrm/mesh.h>
#include <vrm/scene.h>
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
  std::shared_ptr<libvrm::gltf::Scene> m_table;

  std::weak_ptr<libvrm::gltf::Node> selected;
  std::weak_ptr<libvrm::gltf::Node> new_selected;

  libvrm::Time NextSpringDelta = libvrm::Time(0.0);
  std::shared_ptr<libvrm::gltf::Scene> m_lastScene;

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

  RuntimeScene(const std::shared_ptr<libvrm::gltf::Scene>& table);
  void Reset();

  std::shared_ptr<RuntimeMesh> GetRuntimeMesh(
    const std::shared_ptr<libvrm::gltf::Mesh>& mesh);
  std::shared_ptr<RuntimeNode> GetRuntimeNode(
    const std::shared_ptr<libvrm::gltf::Node>& node);

  std::shared_ptr<RuntimeSpringJoint> GetRuntimeJoint(
    const std::shared_ptr<libvrm::vrm::SpringJoint>& joint);
  std::shared_ptr<RuntimeSpringCollision> GetRuntimeSpringCollision(
    const std::shared_ptr<libvrm::vrm::SpringBone>& springBone);

  void Render(const std::shared_ptr<libvrm::gltf::Scene>& scene,
              const RenderFunc& render,
              libvrm::IGizmoDrawer* gizmo);

  void SpringUpdate(const std::shared_ptr<libvrm::vrm::SpringBone>& solver,
                    libvrm::Time deltaForSimulation);
  void SpringDrawGizmo(const std::shared_ptr<libvrm::vrm::SpringBone>& solver,
                       libvrm::IGizmoDrawer* gizmo);
  void SpringColliderDrawGizmo(
    const std::shared_ptr<libvrm::vrm::SpringCollider>& collider,
    libvrm::IGizmoDrawer* gizmo);
  DirectX::XMVECTOR SpringColliderPosition(
    const std::shared_ptr<libvrm::vrm::SpringCollider>& collider);

  // vrm::HumanPose UpdateHumanPose();
  // void SetHumanPose(const vrm::HumanPose& pose);
  // void SyncHierarchy();
}  // void SetInitialPose()
  // {
  //   for (auto& node : m_nodes) {
  //     node->Transform = node->InitialTransform;
  //   }
  //   for (auto& root : m_roots) {
  //     root->CalcWorldMatrix(true);
  //   }
  //   RaiseSceneUpdated();
  // }

;

}
