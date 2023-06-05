#pragma once
#include "../gltfroot.h"
#include "../humanoid/humanpose.h"
#include "runtime_springjoint.h"
#include "spring_bone.h"
#include <unordered_map>

namespace libvrm {
struct Skin;
}

namespace runtimescene {

template<typename T>
inline std::optional<size_t>
_IndexOf(std::span<const T> values, const T& target)
{
  for (size_t i = 0; i < values.size(); ++i) {
    if (values[i] == target) {
      return i;
    }
  }
  return {};
}

struct RuntimeNode;
struct DeformedMesh;
struct RuntimeSpringCollision;
struct BaseMesh;
struct Animation;

using RenderFunc = std::function<
  void(const std::shared_ptr<BaseMesh>&, const DeformedMesh&, const float[16])>;

struct RuntimeScene
{
  std::shared_ptr<libvrm::GltfRoot> m_table;
  std::vector<std::shared_ptr<RuntimeNode>> m_nodes;
  std::vector<std::shared_ptr<RuntimeNode>> m_roots;
  std::vector<std::shared_ptr<BaseMesh>> m_meshes;
  std::vector<std::shared_ptr<libvrm::Skin>> m_skins;
  std::vector<std::shared_ptr<Animation>> m_animations;

  std::optional<size_t> IndexOf(const std::shared_ptr<RuntimeNode>& node) const
  {
    return _IndexOf<std::shared_ptr<RuntimeNode>>(m_nodes, node);
  }

  std::list<std::function<void(const RuntimeScene& scene)>> m_sceneUpdated;
  void RaiseSceneUpdated()
  {
    for (auto& callback : m_sceneUpdated) {
      callback(*this);
    }
  }

  libvrm::Time NextSpringDelta = libvrm::Time(0.0);
  std::shared_ptr<libvrm::GltfRoot> m_lastScene;

  std::unordered_map<uint32_t, std::shared_ptr<DeformedMesh>> m_meshMap;
  std::unordered_map<std::shared_ptr<libvrm::Node>,
                     std::shared_ptr<RuntimeNode>>
    m_nodeMap;

  std::unordered_map<std::shared_ptr<libvrm::SpringJoint>,
                     std::shared_ptr<RuntimeSpringJoint>>
    m_jointMap;
  std::unordered_map<std::shared_ptr<libvrm::SpringBone>,
                     std::shared_ptr<RuntimeSpringCollision>>
    m_springCollisionMap;

  RuntimeScene(const std::shared_ptr<libvrm::GltfRoot>& table);
  void Reset();

  std::shared_ptr<DeformedMesh> GetDeformedMesh(uint32_t mesh);
  std::shared_ptr<RuntimeNode> GetRuntimeNode(
    const std::shared_ptr<libvrm::Node>& node);

  std::shared_ptr<RuntimeSpringJoint> GetRuntimeJoint(
    const std::shared_ptr<libvrm::SpringJoint>& joint);
  std::shared_ptr<RuntimeSpringCollision> GetRuntimeSpringCollision(
    const std::shared_ptr<libvrm::SpringBone>& springBone);

  std::vector<libvrm::DrawItem> m_drawables;
  std::span<const libvrm::DrawItem> Drawables();

  std::vector<DirectX::XMFLOAT4X4> m_shapeMatrices;
  std::span<const DirectX::XMFLOAT4X4> ShapeMatrices();

  void SpringUpdate(const std::shared_ptr<libvrm::SpringBone>& solver,
                    libvrm::Time deltaForSimulation);
  void SpringDrawGizmo(const std::shared_ptr<libvrm::SpringBone>& solver,
                       libvrm::IGizmoDrawer* gizmo);
  void SpringColliderDrawGizmo(
    const std::shared_ptr<libvrm::SpringCollider>& collider,
    libvrm::IGizmoDrawer* gizmo);
  DirectX::XMVECTOR SpringColliderPosition(
    const std::shared_ptr<libvrm::SpringCollider>& collider);

  void NodeConstraintProcess(const libvrm::NodeConstraint& constraint,
                             const std::shared_ptr<RuntimeNode>& dst);

  // humanpose
  std::vector<libvrm::HumanBones> m_humanBoneMap;
  std::vector<DirectX::XMFLOAT4> m_rotations;
  libvrm::HumanPose m_pose;
  libvrm::HumanPose UpdateHumanPose();
  void SetHumanPose(const libvrm::HumanPose& pose);
  void SyncHierarchy();
  void DrawGizmo(libvrm::IGizmoDrawer* gizmo);
};

}
