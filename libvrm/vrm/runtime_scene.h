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

inline DirectX::XMFLOAT3
ToVec3(const gltfjson::tree::NodePtr& json)
{
  DirectX::XMFLOAT3 v3;
  if (auto a = json->Array()) {
    int i = 0;
    for (auto v : *a) {
      if (auto p = v->Ptr<float>()) {
        (&v3.x)[i++] = *p;
      }
    }
  }
  return v3;
}

inline DirectX::XMFLOAT4
ToVec4(const gltfjson::tree::NodePtr& json)
{
  DirectX::XMFLOAT4 v4;
  if (auto a = json->Array()) {
    int i = 0;
    for (auto v : *a) {
      if (auto p = v->Ptr<float>()) {
        (&v4.x)[i++] = *p;
      }
    }
  }
  return v4;
}

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
  std::shared_ptr<GltfRoot> m_base;
  std::vector<std::shared_ptr<RuntimeNode>> m_nodes;
  std::vector<std::shared_ptr<RuntimeNode>> m_roots;
  std::vector<std::shared_ptr<Animation>> m_animations;
  std::shared_ptr<Timeline> m_timeline;
  std::unordered_map<uint32_t, std::vector<float>> m_moprhWeigts;

  // extensions
  std::shared_ptr<Expressions> m_expressions;
  // spring
  std::vector<std::shared_ptr<SpringCollider>> m_springColliders;
  std::vector<std::shared_ptr<SpringColliderGroup>> m_springColliderGroups;
  std::vector<std::shared_ptr<SpringBone>> m_springBones;
  std::shared_ptr<libvrm::SpringBone> m_springBoneSelected;
  std::shared_ptr<libvrm::SpringJoint> m_springJointSelected;

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

  HumanPose m_pose;

  RuntimeScene(const std::shared_ptr<GltfRoot>& table);
  void Reset();

  void SetActiveAnimation(uint32_t index);

  void SetMorphWeights(uint32_t nodeIndex, std::span<const float> values);

  std::shared_ptr<RuntimeNode> GetBoneNode(HumanBones bone);

  std::shared_ptr<RuntimeSpringJoint> GetOrCreateRuntimeJoint(
    const std::shared_ptr<SpringJoint>& joint);
  std::shared_ptr<RuntimeSpringCollision> GetOrCreateRuntimeSpringCollision(
    const std::shared_ptr<SpringBone>& springBone);

  void UpdateNodeStates(std::span<boneskin::NodeState> nodestates);

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
  HumanPose UpdateHumanPose();
  HumanPose CurrentHumanPose() const { return m_pose; }
  void SetHumanPose(const HumanPose& pose);
  void SyncHierarchy();
  void DrawGizmo(IGizmoDrawer* gizmo);

  std::string CopyVrmPoseText();

  std::shared_ptr<RuntimeNode> GetSelectedNode() const;
  void SelectNode(const std::shared_ptr<libvrm::RuntimeNode>& node);
  bool IsSelected(const std::shared_ptr<libvrm::RuntimeNode>& node) const;

  void SelectJoint(const std::shared_ptr<libvrm::SpringJoint>& joint);
};

} // namespace
