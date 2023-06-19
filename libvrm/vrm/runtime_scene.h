#pragma once
#include "gltfroot.h"
#include "humanoid/humanpose.h"
#include "runtime_springjoint.h"
#include "spring_bone.h"
#include "vrm/expression.h"
#include <unordered_map>

namespace libvrm {
struct RuntimeNode;
struct RuntimeSpringCollision;
struct Animation;

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

struct RuntimeScene
{
  std::shared_ptr<GltfRoot> m_table;
  std::vector<std::shared_ptr<RuntimeNode>> m_nodes;
  std::vector<std::shared_ptr<RuntimeNode>> m_roots;
  std::shared_ptr<RuntimeNode> m_selected;
  std::vector<std::shared_ptr<Animation>> m_animations;
  std::shared_ptr<Timeline> m_timeline;
  std::unordered_map<uint32_t, std::vector<float>> m_moprhWeigts;

  // extensions
  std::shared_ptr<Expressions> m_expressions;
  // spring
  std::vector<std::shared_ptr<SpringCollider>> m_springColliders;
  std::vector<std::shared_ptr<SpringColliderGroup>> m_springColliderGroups;
  std::vector<std::shared_ptr<SpringBone>> m_springBones;

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

  Time NextSpringDelta = libvrm::Time(0.0);
  std::shared_ptr<GltfRoot> m_lastScene;

  std::unordered_map<std::shared_ptr<SpringJoint>,
                     std::shared_ptr<RuntimeSpringJoint>>
    m_jointMap;
  std::unordered_map<std::shared_ptr<SpringBone>,
                     std::shared_ptr<RuntimeSpringCollision>>
    m_springCollisionMap;

  RuntimeScene(const std::shared_ptr<GltfRoot>& table);
  void Reset();

  void SetActiveAnimation(uint32_t index);

  void SetMorphWeights(uint32_t nodeIndex, std::span<const float> values);

  std::shared_ptr<RuntimeNode> GetBoneNode(HumanBones bone);

  std::shared_ptr<RuntimeSpringJoint> GetRuntimeJoint(
    const std::shared_ptr<SpringJoint>& joint);
  std::shared_ptr<RuntimeSpringCollision> GetRuntimeSpringCollision(
    const std::shared_ptr<SpringBone>& springBone);

  void UpdateDrawables(std::span<DrawItem> drawables);

  std::vector<DirectX::XMFLOAT4X4> m_shapeMatrices;
  std::span<const DirectX::XMFLOAT4X4> ShapeMatrices();

  void SpringUpdate(const std::shared_ptr<SpringBone>& solver,
                    Time deltaForSimulation);
  void SpringDrawGizmo(const std::shared_ptr<SpringBone>& solver,
                       IGizmoDrawer* gizmo);
  void SpringColliderDrawGizmo(const std::shared_ptr<SpringCollider>& collider,
                               IGizmoDrawer* gizmo);
  DirectX::XMVECTOR SpringColliderPosition(
    const std::shared_ptr<SpringCollider>& collider);

  void NodeConstraintProcess(const NodeConstraint& constraint,
                             const std::shared_ptr<RuntimeNode>& dst);

  // humanpose
  std::vector<HumanBones> m_humanBoneMap;
  std::vector<DirectX::XMFLOAT4> m_rotations;
  HumanPose m_pose;
  HumanPose UpdateHumanPose();
  void SetHumanPose(const HumanPose& pose);
  void SyncHierarchy();
  void DrawGizmo(IGizmoDrawer* gizmo);

  std::string CopyVrmPoseText();
};

} // namespace
