#include "vrm/runtimescene/scene.h"
#include "vrm/runtimescene/mesh.h"
#include "vrm/runtimescene/node.h"
#include "vrm/runtimescene/springcollision.h"
#include <vrm/skin.h>

namespace runtimescene {

RuntimeScene::RuntimeScene(const std::shared_ptr<libvrm::gltf::Scene>& table)
  : m_table(table)
{
  Reset();
}

void
RuntimeScene::Reset()
{
  m_nodeMap.clear();
  m_nodes.clear();
  m_roots.clear();

  // COPY hierarchy
  for (auto& node : m_table->m_nodes) {
    auto runtime = std::make_shared<RuntimeNode>(node);
    m_nodeMap.insert({ node, runtime });
    m_nodes.push_back(runtime);
  }

  for (auto& node : m_table->m_nodes) {
    auto runtime = GetRuntimeNode(node);
    if (auto parent = node->Parent.lock()) {
      auto runtimeParent = GetRuntimeNode(parent);
      RuntimeNode::AddChild(runtimeParent, runtime);
    } else {
      m_roots.push_back(runtime);
    }
  }
}

std::shared_ptr<RuntimeNode>
RuntimeScene::GetRuntimeNode(const std::shared_ptr<libvrm::gltf::Node>& node)
{
  auto found = m_nodeMap.find(node);
  if (found != m_nodeMap.end()) {
    return found->second;
  }

  assert(false);
  return {};
}

std::shared_ptr<RuntimeMesh>
RuntimeScene::GetRuntimeMesh(const std::shared_ptr<libvrm::gltf::Mesh>& mesh)
{
  auto found = m_meshMap.find(mesh);
  if (found != m_meshMap.end()) {
    return found->second;
  }

  auto runtime = std::make_shared<RuntimeMesh>(mesh);
  m_meshMap.insert({ mesh, runtime });
  return runtime;
}

std::shared_ptr<RuntimeSpringJoint>
RuntimeScene::GetRuntimeJoint(
  const std::shared_ptr<libvrm::vrm::SpringJoint>& joint)
{
  auto found = m_jointMap.find(joint);
  if (found != m_jointMap.end()) {
    return found->second;
  }

  auto runtime = std::make_shared<RuntimeSpringJoint>(joint);
  m_jointMap.insert({ joint, runtime });
  return runtime;
}

std::shared_ptr<RuntimeSpringCollision>
RuntimeScene::GetRuntimeSpringCollision(
  const std::shared_ptr<libvrm::vrm::SpringBone>& springBone)
{
  auto found = m_springCollisionMap.find(springBone);
  if (found != m_springCollisionMap.end()) {
    return found->second;
  }

  auto runtime = std::make_shared<RuntimeSpringCollision>(springBone);
  m_springCollisionMap.insert({ springBone, runtime });
  return runtime;
}

std::span<const libvrm::gltf::DrawItem>
RuntimeScene::Drawables()
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
  for (auto& node : m_table->m_nodes) {
    if (auto constraint = node->Constraint) {
      constraint->Process(node);
    }
  }
  for (auto& root : m_table->m_roots) {
    GetRuntimeNode(root)->CalcWorldMatrix(true);
  }

  // springbone
  for (auto& spring : m_table->m_springBones) {
    SpringUpdate(spring, NextSpringDelta);
  }
  NextSpringDelta = {};

  if (m_table->m_expressions) {
    // VRM0 expression to morphTarget
    auto nodeToIndex = [nodes = m_table->m_nodes,
                        expressions = m_table->m_expressions](
                         const std::shared_ptr<libvrm::gltf::Node>& node) {
      for (uint32_t i = 0; i < nodes.size(); ++i) {
        if (node == nodes[i]) {
          return i;
        }
      }
      return (uint32_t)-1;
    };
    for (auto& [k, v] :
         m_table->m_expressions->EvalMorphTargetMap(nodeToIndex)) {
      auto& morph_node = m_table->m_nodes[k.NodeIndex];
      auto instance = GetRuntimeMesh(morph_node->Mesh);
      instance->weights[k.MorphIndex] = v;
    }
  }

  // skinning
  for (auto& node : m_table->m_nodes) {
    if (node->Mesh) {
      // auto mesh = m_table->m_meshes[*mesh_index];

      // mesh animation
      std::span<DirectX::XMFLOAT4X4> skinningMatrices;

      // skinning
      if (auto skin = node->Skin) {
        skin->CurrentMatrices.resize(skin->BindMatrices.size());

        auto rootInverse = DirectX::XMMatrixIdentity();
        if (auto root_index = skin->Root) {
          rootInverse = DirectX::XMMatrixInverse(
            nullptr, GetRuntimeNode(node)->WorldMatrix());
        }

        for (int i = 0; i < skin->Joints.size(); ++i) {
          auto node = m_table->m_nodes[skin->Joints[i]];
          auto m = skin->BindMatrices[i];
          DirectX::XMStoreFloat4x4(&skin->CurrentMatrices[i],
                                   DirectX::XMLoadFloat4x4(&m) *
                                     GetRuntimeNode(node)->WorldMatrix() *
                                     rootInverse);
        }

        skinningMatrices = skin->CurrentMatrices;
      }

      // apply morphtarget & skinning
      auto instance = GetRuntimeMesh(node->Mesh);
      instance->applyMorphTargetAndSkinning(*node->Mesh, skinningMatrices);
    }
  }

  m_drawables.clear();
  for (auto& node : m_table->m_nodes) {
    if (node->Mesh) {
      m_drawables.push_back({
        .Mesh = node->Mesh,
      });
      DirectX::XMStoreFloat4x4(&m_drawables.back().Matrix,
                               GetRuntimeNode(node)->WorldMatrix());
      auto instance = GetRuntimeMesh(node->Mesh);
    }
  }

  return m_drawables;
}

std::span<const DirectX::XMFLOAT4X4>
RuntimeScene::ShapeMatrices()
{
  m_shapeMatrices.clear();
  for (auto& node : m_table->m_nodes) {
    m_shapeMatrices.push_back({});
    auto shape = DirectX::XMLoadFloat4x4(&node->ShapeMatrix);
    DirectX::XMStoreFloat4x4(&m_shapeMatrices.back(),
                             shape * GetRuntimeNode(node)->WorldMatrix());
  }
  return m_shapeMatrices;
}

void
RuntimeScene::DrawGizmo(libvrm::IGizmoDrawer* gizmo)
{
  for (auto& spring : m_table->m_springBones) {
    SpringDrawGizmo(spring, gizmo);
  }
  for (auto& collider : m_table->m_springColliders) {
    SpringColliderDrawGizmo(collider, gizmo);
  }
}

void
RuntimeScene::SpringUpdate(
  const std::shared_ptr<libvrm::vrm::SpringBone>& spring,
  libvrm::Time delta)
{
  bool doUpdate = delta.count() > 0;
  if (!doUpdate) {
    return;
  }

  auto collision = GetRuntimeSpringCollision(spring);
  for (auto& joint : spring->Joints) {
    collision->Clear();
    auto runtime = GetRuntimeJoint(joint);
    runtime->Update(this, delta, collision.get());
  }
}

void
RuntimeScene::SpringDrawGizmo(
  const std::shared_ptr<libvrm::vrm::SpringBone>& solver,
  libvrm::IGizmoDrawer* gizmo)
{
  for (auto& joint : solver->Joints) {
    auto runtime = GetRuntimeJoint(joint);
    runtime->DrawGizmo(this, gizmo);
  }
}

void
RuntimeScene::SpringColliderDrawGizmo(
  const std::shared_ptr<libvrm::vrm::SpringCollider>& collider,
  libvrm::IGizmoDrawer* gizmo)
{
  DirectX::XMFLOAT3 offset;
  DirectX::XMStoreFloat3(
    &offset,
    GetRuntimeNode(collider->Node)
      ->WorldTransformPoint(DirectX::XMLoadFloat3(&collider->Offset)));
  switch (collider->Type) {
    case libvrm::vrm::SpringColliderShapeType::Sphere:
      gizmo->DrawSphere(offset, collider->Radius, { 0, 1, 1, 1 });
      break;

    case libvrm::vrm::SpringColliderShapeType::Capsule: {
      DirectX::XMFLOAT3 tail;
      DirectX::XMStoreFloat3(
        &tail,
        GetRuntimeNode(collider->Node)
          ->WorldTransformPoint(DirectX::XMLoadFloat3(&collider->Tail)));

      // gizmo->DrawSphere(offset, Radius, { 0, 1, 1, 1 });
      // gizmo->DrawSphere(tail, Radius, { 0, 1, 1, 1 });
      gizmo->DrawCapsule(offset, tail, collider->Radius, { 0, 1, 1, 1 });
      break;
    }
  }
}

DirectX::XMVECTOR
RuntimeScene::SpringColliderPosition(
  const std::shared_ptr<libvrm::vrm::SpringCollider>& collider)
{
  return GetRuntimeNode(collider->Node)
    ->WorldTransformPoint(DirectX::XMLoadFloat3(&collider->Offset));
}

libvrm::vrm::HumanPose
RuntimeScene::UpdateHumanPose()
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
    if (auto humanoid = node->Node->Humanoid) {
      m_humanBoneMap.push_back(*humanoid);
      if (m_humanBoneMap.back() == libvrm::vrm::HumanBones::hips) {
        // delta move
        DirectX::XMStoreFloat3(
          &m_pose.RootPosition,
          DirectX::XMVectorSubtract(
            DirectX::XMLoadFloat3(&node->WorldTransform.Translation),
            DirectX::XMLoadFloat3(
              &node->Node->WorldInitialTransform.Translation)));
      }

      // retarget
      auto normalized = mult4(
        DirectX::XMQuaternionInverse(
          DirectX::XMLoadFloat4(&node->Node->WorldInitialTransform.Rotation)),
        DirectX::XMLoadFloat4(&node->Transform.Rotation),
        DirectX::XMQuaternionInverse(
          DirectX::XMLoadFloat4(&node->Node->InitialTransform.Rotation)),
        DirectX::XMLoadFloat4(&node->Node->WorldInitialTransform.Rotation));

      m_rotations.push_back({});
      DirectX::XMStoreFloat4(&m_rotations.back(), normalized);
    }
  }
  m_pose.Bones = m_humanBoneMap;
  m_pose.Rotations = m_rotations;
  return m_pose;
}

void
RuntimeScene::SetHumanPose(const libvrm::vrm::HumanPose& pose)
{
  assert(pose.Bones.size() == pose.Rotations.size());

  for (int i = 0; i < pose.Bones.size(); ++i) {
    if (auto init = m_table->GetBoneNode(pose.Bones[i])) {
      auto node = GetRuntimeNode(init);
      if (i == 0) {
        DirectX::XMStoreFloat3(
          &node->Transform.Translation,
          DirectX::XMVectorAdd(
            DirectX::XMLoadFloat3(&init->InitialTransform.Translation),
            DirectX::XMLoadFloat3(&pose.RootPosition)));
      }

      auto worldInitial =
        DirectX::XMLoadFloat4(&init->WorldInitialTransform.Rotation);
      auto q = DirectX::XMLoadFloat4(&pose.Rotations[i]);
      auto worldInitialInv = DirectX::XMQuaternionInverse(worldInitial);
      auto localInitial =
        DirectX::XMLoadFloat4(&init->InitialTransform.Rotation);

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
RuntimeScene::SyncHierarchy()
{
  for (auto& root : m_roots) {
    root->CalcWorldMatrix(true);
  }
}

}
