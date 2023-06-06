#include "runtime_scene.h"
#include "animation.h"
#include "deformed_mesh.h"
#include "runtime_node.h"
#include "spring_collision.h"
#include <gltfjson.h>

namespace libvrm {

static std::expected<std::shared_ptr<Animation>, std::string>
ParseAnimation(const gltfjson::Root& root, const gltfjson::Bin& bin, int i)
{
  auto animation = root.Animations[i];
  auto ptr = std::make_shared<Animation>(animation.Name());

  // samplers
  auto& samplers = animation.Samplers;

  // channels
  auto& channels = animation.Channels;
  for (auto channel : channels) {
    int sampler_index = *channel.Sampler();
    auto sampler = samplers[sampler_index];

    auto target = channel.Target();
    int node_index = *target->Node();
    auto path = target->Path();

    // time
    int input_index = *sampler.Input();
    if (auto times = bin.GetAccessorBytes<float>(root, input_index)) {
      int output_index = *sampler.Output();
      if (path == u8"translation") {
        if (auto values =
              bin.GetAccessorBytes<DirectX::XMFLOAT3>(root, output_index)) {
          ptr->AddTranslation(node_index,
                              *times,
                              *values,
                              root.Nodes[node_index].Name() + u8"-translation");
        } else {
          return std::unexpected{ values.error() };
        }
      } else if (path == u8"rotation") {
        if (auto values =
              bin.GetAccessorBytes<DirectX::XMFLOAT4>(root, output_index)) {
          ptr->AddRotation(node_index,
                           *times,
                           *values,
                           root.Nodes[node_index].Name() + u8"-rotation");
        } else {
          return std::unexpected{ values.error() };
        }
      } else if (path == u8"scale") {
        if (auto values =
              bin.GetAccessorBytes<DirectX::XMFLOAT3>(root, output_index)) {
          ptr->AddScale(node_index,
                        *times,
                        *values,
                        root.Nodes[node_index].Name() + u8"-scale");
        } else {
          return std::unexpected{ values.error() };
        }
      } else if (path == u8"weights") {
        if (auto values = bin.GetAccessorBytes<float>(root, output_index)) {
          auto node = root.Nodes[node_index];
          if (auto mesh = node.Mesh()) {
            if (values->size() !=
                root.Meshes[*mesh].Primitives[0].Targets.size() *
                  times->size()) {
              return std::unexpected{ "animation-weights: size not match" };
            }
            ptr->AddWeights(
              node_index, *times, *values, node.Name() + u8"-weights");
          } else {
            return std::unexpected{ "animation-weights: no node.mesh" };
          }
        } else {
          return std::unexpected{ values.error() };
        }
      } else {
        return std::unexpected{ "animation path is not implemented: " };
      }
    } else {
      return std::unexpected{ times.error() };
    }
  }

  return ptr;
}

RuntimeScene::RuntimeScene(const std::shared_ptr<libvrm::GltfRoot>& table)
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
RuntimeScene::GetRuntimeNode(const std::shared_ptr<libvrm::Node>& node)
{
  auto found = m_nodeMap.find(node);
  if (found != m_nodeMap.end()) {
    return found->second;
  }

  assert(false);
  return {};
}

std::shared_ptr<RuntimeSpringJoint>
RuntimeScene::GetRuntimeJoint(const std::shared_ptr<libvrm::SpringJoint>& joint)
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
  const std::shared_ptr<libvrm::SpringBone>& springBone)
{
  auto found = m_springCollisionMap.find(springBone);
  if (found != m_springCollisionMap.end()) {
    return found->second;
  }

  auto runtime = std::make_shared<RuntimeSpringCollision>(springBone);
  m_springCollisionMap.insert({ springBone, runtime });
  return runtime;
}

void
RuntimeScene::UpdateDrawables(std::span<DrawItem> drawables)
{
  if (!m_table->m_gltf) {
    return;
  }

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
    if (auto constraint = node->Node->Constraint) {
      NodeConstraintProcess(*constraint, node);
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
                         const std::shared_ptr<libvrm::Node>& node) {
      for (uint32_t i = 0; i < nodes.size(); ++i) {
        if (node == nodes[i]) {
          return i;
        }
      }
      return (uint32_t)-1;
    };
    for (auto& [k, v] :
         m_table->m_expressions->EvalMorphTargetMap(nodeToIndex)) {
      auto& item = drawables[k.NodeIndex];
      item.MorphMap[k.MorphIndex] = v;
    }
  }

  for (uint32_t i = 0; i < drawables.size(); ++i) {
    // model matrix
    DirectX::XMStoreFloat4x4(
      &drawables[i].Matrix, GetRuntimeNode(m_table->m_nodes[i])->WorldMatrix());
  }
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
RuntimeScene::SpringUpdate(const std::shared_ptr<libvrm::SpringBone>& spring,
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
RuntimeScene::SpringDrawGizmo(const std::shared_ptr<libvrm::SpringBone>& solver,
                              libvrm::IGizmoDrawer* gizmo)
{
  for (auto& joint : solver->Joints) {
    auto runtime = GetRuntimeJoint(joint);
    runtime->DrawGizmo(this, gizmo);
  }
}

void
RuntimeScene::SpringColliderDrawGizmo(
  const std::shared_ptr<libvrm::SpringCollider>& collider,
  libvrm::IGizmoDrawer* gizmo)
{
  DirectX::XMFLOAT3 offset;
  DirectX::XMStoreFloat3(
    &offset,
    GetRuntimeNode(collider->Node)
      ->WorldTransformPoint(DirectX::XMLoadFloat3(&collider->Offset)));
  switch (collider->Type) {
    case libvrm::SpringColliderShapeType::Sphere:
      gizmo->DrawSphere(offset, collider->Radius, { 0, 1, 1, 1 });
      break;

    case libvrm::SpringColliderShapeType::Capsule: {
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
  const std::shared_ptr<libvrm::SpringCollider>& collider)
{
  return GetRuntimeNode(collider->Node)
    ->WorldTransformPoint(DirectX::XMLoadFloat3(&collider->Offset));
}

libvrm::HumanPose
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
      if (m_humanBoneMap.back() == libvrm::HumanBones::hips) {
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
RuntimeScene::SetHumanPose(const libvrm::HumanPose& pose)
{
  assert(pose.Bones.size() == pose.Rotations.size());

  for (int i = 0; i < pose.Bones.size(); ++i) {
    if (auto init = m_table->GetBoneNode(pose.Bones[i])) {
      auto node = GetRuntimeNode(init);
      if (i == 0) {
        // hips move
        // DirectX::XMStoreFloat3(
        //   &node->Transform.Translation,
        //   DirectX::XMVectorAdd(
        //     DirectX::XMLoadFloat3(&init->InitialTransform.Translation),
        //     DirectX::XMLoadFloat3(&pose.RootPosition)));

        // TODO: position from Model Root ?
        auto pos = pose.RootPosition;
        auto init_pos = init->WorldInitialTransform.Translation;
        node->SetWorldMatrix(DirectX::XMMatrixTranslation(
          init_pos.x + pos.x, init_pos.y + pos.y, init_pos.z + pos.z));
        auto a = 0;
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

// static void
// Constraint_Aim(const std::shared_ptr<Node>& src,
//                const std::shared_ptr<Node>& dst,
//                float weight,
//                const DirectX::XMVECTOR axis)
// {
//   auto dstParentWorldQuat = dst->ParentWorldRotation();
//   auto fromVec = DirectX::XMVector3Rotate(
//     axis,
//     DirectX::XMQuaternionMultiply(
//       DirectX::XMLoadFloat4(&dst->InitialTransform.Rotation),
//       dstParentWorldQuat));
//   auto toVec = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(
//     DirectX::XMLoadFloat3(&src->WorldTransform.Translation),
//     DirectX::XMLoadFloat3(&dst->WorldTransform.Translation)));
//   auto fromToQuat = dmath::rotate_from_to(fromVec, toVec);
//
//   DirectX::XMStoreFloat4(
//     &dst->Transform.Rotation,
//     DirectX::XMQuaternionSlerp(
//       DirectX::XMLoadFloat4(&dst->InitialTransform.Rotation),
//       mul4(DirectX::XMLoadFloat4(&dst->InitialTransform.Rotation),
//            dstParentWorldQuat,
//            fromToQuat,
//            DirectX::XMQuaternionInverse(dstParentWorldQuat)),
//       weight));
// }

// static void
// Constraint_Roll(const std::shared_ptr<Node>& src,
//                 const std::shared_ptr<Node>& dst,
//                 float weight,
//                 DirectX::XMVECTOR axis)
// {
//   auto deltaSrcQuat = DirectX::XMQuaternionMultiply(
//     DirectX::XMLoadFloat4(&src->Transform.Rotation),
//     DirectX::XMQuaternionInverse(
//       DirectX::XMLoadFloat4(&src->InitialTransform.Rotation)));
//   auto deltaSrcQuatInParent =
//     mul3(DirectX::XMQuaternionInverse(
//            DirectX::XMLoadFloat4(&src->InitialTransform.Rotation)),
//          deltaSrcQuat,
//          DirectX::XMLoadFloat4(&src->InitialTransform.Rotation));
//   auto deltaSrcQuatInDst =
//     mul3(DirectX::XMLoadFloat4(&dst->InitialTransform.Rotation),
//          deltaSrcQuatInParent,
//          DirectX::XMQuaternionInverse(
//            DirectX::XMLoadFloat4(&dst->InitialTransform.Rotation)));
//   auto toVec = DirectX::XMQuaternionMultiply(axis, deltaSrcQuatInDst);
//   auto fromToQuat = dmath::rotate_from_to(axis, toVec);
//
//   DirectX::XMStoreFloat4(
//     &dst->Transform.Rotation,
//     DirectX::XMQuaternionSlerp(
//       DirectX::XMLoadFloat4(&dst->InitialTransform.Rotation),
//       DirectX::XMQuaternionMultiply(DirectX::XMQuaternionInverse(fromToQuat),
//                                     deltaSrcQuatInDst),
//       weight));
// }

void
RuntimeScene::NodeConstraintProcess(const libvrm::NodeConstraint& constraint,
                                    const std::shared_ptr<RuntimeNode>& dst)
{
  auto src = constraint.Source.lock();
  if (!src) {
    return;
  }

  // switch (Type) {
  //   case NodeConstraintTypes::Rotation:
  //     Constraint_Rotation(src, dst, Weight);
  //     break;
  //
  //   case NodeConstraintTypes::Roll:
  //     Constraint_Roll(src, dst, Weight, GetRollVector(RollAxis));
  //     break;
  //
  //   case NodeConstraintTypes::Aim:
  //     Constraint_Aim(src, dst, Weight, GetAxisVector(AimAxis));
  //     break;
  // }
}
}
