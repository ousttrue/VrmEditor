#include "runtime_scene.h"
#include "animation.h"
#include "dmath.h"
#include "expression.h"
#include "gizmo.h"
#include "runtime_node.h"
#include "spring_collision.h"
#include <gltfjson.h>
#include <gltfjson/gltf_typing_vrm0.h>
#include <gltfjson/gltf_typing_vrm1.h>

namespace libvrm {

static void
ParseVrm0(RuntimeScene* scene, const gltfjson::vrm0::VRM& VRM)
{
  // meta
  // specVersion
  // exporterVersion
  // firstPerson

  // if (has(VRM, "blendShapeMaster")) {
  //
  //   scene->m_expressions = std::make_shared<vrm::Expressions>();
  //
  //   auto& blendShapeMaster = VRM.at("blendShapeMaster");
  //   if (has(blendShapeMaster, "blendShapeGroups")) {
  //     auto& blendShapeGroups = blendShapeMaster.at("blendShapeGroups");
  //     for (auto& g : blendShapeGroups) {
  //       //
  //       {"binds":[],"isBinary":false,"materialValues":[],"name":"Neutral","presetName":"neutral"}
  //       // std::cout << g << std::endl;
  //       auto expression = scene->m_expressions->addBlendShape(
  //         g.at("presetName"), g.at("name"), g.value("isBinary", false));
  //       if (has(g, "binds")) {
  //         for (vrm::ExpressionMorphTargetBind bind : g.at("binds")) {
  //           // [0-100] to [0-1]
  //           bind.weight *= 0.01f;
  //           for (auto& node : scene->m_nodes) {
  //             if (node->Mesh == scene->m_meshes[bind.mesh]) {
  //               bind.Node = node;
  //               break;
  //             }
  //           }
  //           expression->morphBinds.push_back(bind);
  //         }
  //       }
  //     }
  //   }
  // }

  if (auto secondaryAnimation = VRM.SecondaryAnimation()) {
    for (auto colliderGroup : secondaryAnimation->ColliderGroups) {
      auto group = std::make_shared<SpringColliderGroup>();
      auto node_index = colliderGroup.NodeId();
      auto colliderNode = scene->m_nodes[*node_index];
      for (auto collider : colliderGroup.Colliders) {
        auto item = std::make_shared<SpringCollider>();
        if (auto offset = collider.Offset()) {
          auto x = (*offset)[u8"x"]->Ptr<float>();
          auto y = (*offset)[u8"y"]->Ptr<float>();
          auto z = (*offset)[u8"z"]->Ptr<float>();
          // vrm0: springbone collider offset is UnityCoordinate(LeftHanded)
          item->Offset = { -*x, *y, *z };
        }
        item->Radius = *collider.Radius();
        item->Node = colliderNode;
        scene->m_springColliders.push_back(item);
        group->Colliders.push_back(item);
      }
      scene->m_springColliderGroups.push_back(group);
    }
    for (auto boneGroup : secondaryAnimation->Springs) {
      auto stiffness = boneGroup.Stifness();
      auto dragForce = boneGroup.DragForce();
      auto radius = boneGroup.HitRadius();
      std::vector<std::shared_ptr<SpringColliderGroup>> colliderGroups;
      if (auto array = boneGroup.ColliderGroups()) {
        for (auto colliderGroup_index : *array) {
          auto colliderGroup =
            scene->m_springColliderGroups[(uint32_t)*colliderGroup_index
                                            ->Ptr<float>()];
          colliderGroups.push_back(colliderGroup);
        }
      }
      if (auto array = boneGroup.Bones()) {
        for (auto bone : *array) {
          auto spring = std::make_shared<SpringBone>();
          spring->AddJointRecursive(
            scene->m_nodes[(uint32_t)*bone->Ptr<float>()],
            *dragForce,
            *stiffness,
            *radius);
          scene->m_springBones.push_back(spring);
          for (auto& g : colliderGroups) {
            spring->AddColliderGroup(g);
          }
        }
      }
    }
  }
}

static void
ParseVrm1(RuntimeScene* scene, const gltfjson::vrm1::VRMC_vrm& VRMC_vrm)
{
  if (auto VRMC_springBone =
        scene->m_table->m_gltf
          ->GetExtension<gltfjson::vrm1::VRMC_springBone>()) {
    // for (auto collider : VRMC_springBone->Colliders) {
    //   auto ptr = std::make_shared<SpringCollider>();
    //   uint32_t node_index = *collider.Node();
    //   ptr->Node = scene->m_nodes[node_index];
    //   if (auto shape = collider.Shape()) {
    //     if (auto sphere = shape->Sphere()) {
    //       ptr->Type = SpringColliderShapeType::Sphere;
    //       ptr->Radius = *sphere->Radius();
    //       // ptr->Offset = *((DirectX::XMFLOAT3*)&gltfjson::Vec3(
    //       //   sphere->m_json->Get(u8"offset"), { 0, 0, 0 }));
    //     } else if (auto capsule = shape->Capsule()) {
    //       ptr->Type = SpringColliderShapeType::Capsule;
    //       ptr->Radius = *capsule->Radius();
    //       // ptr->Offset = capsule.value("offset", DirectX::XMFLOAT3{ 0, 0, 0
    //       // }); ptr->Tail = capsule.value("tail", DirectX::XMFLOAT3{ 0, 0, 0
    //       // });
    //     } else {
    //       assert(false);
    //     }
    //   }
    //   scene->m_springColliders.push_back(ptr);
    // }
    // for (auto colliderGroup : VRMC_springBone->ColliderGroups) {
    //   auto ptr = std::make_shared<SpringColliderGroup>();
    //   // for (auto collider : colliderGroup.Colliders) {
    //   //   auto collider_index =
    //   // ptr->Colliders.push_back(scene->m_springColliders[collider_index]);
    //   // }
    //   scene->m_springColliderGroups.push_back(ptr);
    // }
    for (auto spring : VRMC_springBone->Springs) {
      auto springBone = std::make_shared<SpringBone>();
      std::shared_ptr<RuntimeNode> head;
      for (auto joint : spring.Joints) {
        auto node_index = (uint32_t)*joint.NodeId();
        auto tail = scene->m_nodes[node_index];
        if (head) {
          float stiffness = *joint.Stiffness();
          float dragForce = *joint.DragForce();
          float radius = *joint.HitRadius();
          springBone->AddJoint(head,
                               tail,
                               tail->Node->InitialTransform.Translation,
                               stiffness,
                               dragForce,
                               radius);
        }
        head = tail;
      }
      scene->m_springBones.push_back(springBone);
    }
  }

  auto& nodes = scene->m_table->m_gltf->Nodes;
  for (size_t i = 0; i < nodes.size(); ++i) {
    auto node = nodes[i];
    auto ptr = scene->m_nodes[i];
    if (auto VRMC_node_constraint =
          node.GetExtension<gltfjson::vrm1::VRMC_node_constraint>()) {
      if (auto constraint = VRMC_node_constraint->Constraint()) {
        static DirectX::XMFLOAT4 s_constraint_color{ 1, 0.6f, 1, 1 };

        if (auto roll = constraint->Roll()) {
          // roll
          auto source_index = roll->SourceId();
          ptr->Constraint = NodeConstraint{
            .Type = NodeConstraintTypes::Roll,
            .Source = scene->m_nodes[*source_index],
            .Weight = *roll->Weight(),
          };
          auto axis = roll->RollAxisString();
          ptr->Constraint->RollAxis =
            NodeConstraintRollAxisFromName(gltfjson::from_u8(axis));
          ptr->Node->ShapeColor = s_constraint_color;
        } else if (auto aim = constraint->Aim()) {
          // aim
          auto source_index = aim->SourceId();
          ptr->Constraint = NodeConstraint{
            .Type = NodeConstraintTypes::Aim,
            .Source = scene->m_nodes[*source_index],
            .Weight = *aim->Weight(),
          };
          auto axis = aim->AimAxisString();
          ptr->Constraint->AimAxis =
            NodeConstraintAimAxisFromName(gltfjson::from_u8(axis));
          ptr->Node->ShapeColor = s_constraint_color;
        } else if (auto rotation = constraint->Rotation()) {
          // rotation
          auto source_index = rotation->SourceId();
          ptr->Constraint = NodeConstraint{
            .Type = NodeConstraintTypes::Rotation,
            .Source = scene->m_nodes[*source_index],
            .Weight = *rotation->Weight(),
          };
          ptr->Node->ShapeColor = s_constraint_color;
        } else {
          assert(false);
        }
      }
    }
  }
}

static std::expected<std::shared_ptr<Animation>, std::string>
ParseAnimation(const gltfjson::Root& root, const gltfjson::Bin& bin, int i)
{
  auto animation = root.Animations[i];
  auto ptr = std::make_shared<Animation>(animation.NameString());

  // samplers
  auto& samplers = animation.Samplers;

  // channels
  auto& channels = animation.Channels;
  for (auto channel : channels) {
    int sampler_index = *channel.SamplerId();
    auto sampler = samplers[sampler_index];

    auto target = channel.Target();
    int node_index = *target->Node();
    auto path = target->PathString();

    // time
    int input_index = *sampler.InputId();
    if (auto times = bin.GetAccessorBytes<float>(root, input_index)) {
      int output_index = *sampler.OutputId();
      if (path == u8"translation") {
        if (auto values =
              bin.GetAccessorBytes<DirectX::XMFLOAT3>(root, output_index)) {
          ptr->AddTranslation(node_index,
                              *times,
                              *values,
                              root.Nodes[node_index].NameString() +
                                u8"-translation");
        } else {
          return std::unexpected{ values.error() };
        }
      } else if (path == u8"rotation") {
        if (auto values =
              bin.GetAccessorBytes<DirectX::XMFLOAT4>(root, output_index)) {
          ptr->AddRotation(node_index,
                           *times,
                           *values,
                           root.Nodes[node_index].NameString() + u8"-rotation");
        } else {
          return std::unexpected{ values.error() };
        }
      } else if (path == u8"scale") {
        if (auto values =
              bin.GetAccessorBytes<DirectX::XMFLOAT3>(root, output_index)) {
          ptr->AddScale(node_index,
                        *times,
                        *values,
                        root.Nodes[node_index].NameString() + u8"-scale");
        } else {
          return std::unexpected{ values.error() };
        }
      } else if (path == u8"weights") {
        if (auto values = bin.GetAccessorBytes<float>(root, output_index)) {
          auto node = root.Nodes[node_index];
          if (auto mesh = node.MeshId()) {
            if (values->size() !=
                root.Meshes[*mesh].Primitives[0].Targets.size() *
                  times->size()) {
              return std::unexpected{ "animation-weights: size not match" };
            }
            ptr->AddWeights(
              node_index, *times, *values, node.NameString() + u8"-weights");
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

RuntimeScene::RuntimeScene(const std::shared_ptr<GltfRoot>& table)
  : m_table(table)
{
  m_timeline = std::make_shared<Timeline>();
  Reset();

  if (table->m_gltf) {
    if (auto VRMC_vrm =
          table->m_gltf->GetExtension<gltfjson::vrm1::VRMC_vrm>()) {
      ParseVrm1(this, *VRMC_vrm);
    } else if (auto VRM = table->m_gltf->GetExtension<gltfjson::vrm0::VRM>()) {
      ParseVrm0(this, *VRM);
    }

    for (int i = 0; i < table->m_gltf->Animations.size(); ++i) {
      if (auto animation = ParseAnimation(*table->m_gltf, table->m_bin, i)) {
        m_animations.push_back(*animation);
      }
    }
  }
}

void
RuntimeScene::SetActiveAnimation(uint32_t index)
{
  if (index >= m_animations.size()) {
    return;
  }

  m_timeline->Tracks.clear();
  auto animation = m_animations[index];
  auto track = m_timeline->AddTrack("gltf", animation->Duration());
  track->Callbacks.push_back([animation, scene = this](auto time, bool repeat) {
    animation->Update(time, *scene, repeat);
    return true;
  });
}

void
RuntimeScene::SetMorphWeights(uint32_t nodeIndex, std::span<const float> values)
{
  auto found = m_moprhWeigts.find(nodeIndex);
  std::vector<float>* pWeights = nullptr;
  if (found != m_moprhWeigts.end()) {
    pWeights = &found->second;
  } else {
    auto inserted = m_moprhWeigts.insert({ nodeIndex, {} });
    pWeights = &inserted.first->second;
  }
  pWeights->assign(values.begin(), values.end());
}

void
RuntimeScene::Reset()
{
  m_nodes.clear();
  m_roots.clear();

  std::unordered_map<std::shared_ptr<Node>, std::shared_ptr<RuntimeNode>>
    nodeMap;

  // COPY hierarchy
  for (auto& node : m_table->m_nodes) {
    auto runtime = std::make_shared<RuntimeNode>(node);
    nodeMap.insert({ node, runtime });
    m_nodes.push_back(runtime);
  }

  for (auto& node : m_table->m_nodes) {
    auto runtime = nodeMap[node];
    if (auto parent = node->Parent.lock()) {
      auto runtimeParent = nodeMap[parent];
      RuntimeNode::AddChild(runtimeParent, runtime);
    } else {
      m_roots.push_back(runtime);
    }
  }
}

std::shared_ptr<RuntimeNode>
RuntimeScene::GetBoneNode(HumanBones bone)
{
  for (auto& node : m_nodes) {
    if (auto humanoid = node->Node->Humanoid) {
      if (*humanoid == bone) {
        return node;
      }
    }
  }
  return {};
}

std::shared_ptr<RuntimeSpringJoint>
RuntimeScene::GetRuntimeJoint(const std::shared_ptr<SpringJoint>& joint)
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
  const std::shared_ptr<SpringBone>& springBone)
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

  // glTF morph animation
  for (int i = 0; i < m_nodes.size(); ++i) {
    auto found = m_moprhWeigts.find(i);
    if (found != m_moprhWeigts.end()) {
      auto& item = drawables[i];
      auto& weights = found->second;
      for (int j = 0; j < weights.size(); ++j) {
        item.MorphMap[j] = weights[j];
      }
    }
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
    if (auto constraint = node->Constraint) {
      NodeConstraintProcess(*constraint, node);
    }
  }
  for (auto& root : m_roots) {
    root->CalcWorldMatrix(true);
  }

  // springbone
  for (auto& spring : m_springBones) {
    SpringUpdate(spring, NextSpringDelta);
  }
  NextSpringDelta = {};

  if (m_expressions) {
    // VRM0 expression to morphTarget
    auto nodeToIndex = [nodes = m_table->m_nodes, expressions = m_expressions](
                         const std::shared_ptr<Node>& node) {
      for (uint32_t i = 0; i < nodes.size(); ++i) {
        if (node == nodes[i]) {
          return i;
        }
      }
      return (uint32_t)-1;
    };
    for (auto& [k, v] : m_expressions->EvalMorphTargetMap(nodeToIndex)) {
      auto& item = drawables[k.NodeIndex];
      item.MorphMap[k.MorphIndex] = v;
    }
  }

  for (uint32_t i = 0; i < drawables.size(); ++i) {
    // model matrix
    DirectX::XMStoreFloat4x4(&drawables[i].Matrix, m_nodes[i]->WorldMatrix());
  }
}

std::span<const DirectX::XMFLOAT4X4>
RuntimeScene::ShapeMatrices()
{
  m_shapeMatrices.clear();
  for (auto& node : m_nodes) {
    m_shapeMatrices.push_back({});
    auto shape = DirectX::XMLoadFloat4x4(&node->Node->ShapeMatrix);
    DirectX::XMStoreFloat4x4(&m_shapeMatrices.back(),
                             shape * node->WorldMatrix());
  }
  return m_shapeMatrices;
}

void
RuntimeScene::DrawGizmo(IGizmoDrawer* gizmo)
{
  for (auto& spring : m_springBones) {
    SpringDrawGizmo(spring, gizmo);
  }
  for (auto& collider : m_springColliders) {
    SpringColliderDrawGizmo(collider, gizmo);
  }
}

void
RuntimeScene::SpringUpdate(const std::shared_ptr<SpringBone>& spring,
                           Time delta)
{
  bool doUpdate = delta.count() > 0;
  if (!doUpdate) {
    return;
  }

  auto collision = GetRuntimeSpringCollision(spring);
  for (auto& joint : spring->Joints) {
    collision->Clear();
    auto runtime = GetRuntimeJoint(joint);
    runtime->Update(delta, collision.get());
  }
}

void
RuntimeScene::SpringDrawGizmo(const std::shared_ptr<SpringBone>& solver,
                              IGizmoDrawer* gizmo)
{
  for (auto& joint : solver->Joints) {
    auto runtime = GetRuntimeJoint(joint);
    runtime->DrawGizmo(gizmo);
  }
}

void
RuntimeScene::SpringColliderDrawGizmo(
  const std::shared_ptr<SpringCollider>& collider,
  IGizmoDrawer* gizmo)
{
  DirectX::XMFLOAT3 offset;
  DirectX::XMStoreFloat3(&offset,
                         collider->Node->WorldTransformPoint(
                           DirectX::XMLoadFloat3(&collider->Offset)));
  switch (collider->Type) {
    case SpringColliderShapeType::Sphere:
      gizmo->DrawSphere(offset, collider->Radius, { 0, 1, 1, 1 });
      break;

    case SpringColliderShapeType::Capsule: {
      DirectX::XMFLOAT3 tail;
      DirectX::XMStoreFloat3(&tail,
                             collider->Node->WorldTransformPoint(
                               DirectX::XMLoadFloat3(&collider->Tail)));

      // gizmo->DrawSphere(offset, Radius, { 0, 1, 1, 1 });
      // gizmo->DrawSphere(tail, Radius, { 0, 1, 1, 1 });
      gizmo->DrawCapsule(offset, tail, collider->Radius, { 0, 1, 1, 1 });
      break;
    }
  }
}

DirectX::XMVECTOR
RuntimeScene::SpringColliderPosition(
  const std::shared_ptr<SpringCollider>& collider)
{
  return collider->Node->WorldTransformPoint(
    DirectX::XMLoadFloat3(&collider->Offset));
}

HumanPose
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
      if (m_humanBoneMap.back() == HumanBones::hips) {
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
RuntimeScene::SetHumanPose(const HumanPose& pose)
{
  assert(pose.Bones.size() == pose.Rotations.size());

  for (int i = 0; i < pose.Bones.size(); ++i) {
    if (auto node = GetBoneNode(pose.Bones[i])) {
      auto init = node->Node;
      if (i == 0) {
        // hips move
        // TODO: position from Model Root ?
        auto pos = pose.RootPosition;
        auto init_pos = init->WorldInitialTransform.Translation;
        node->SetWorldMatrix(DirectX::XMMatrixTranslation(
          init_pos.x + pos.x, init_pos.y + pos.y, init_pos.z + pos.z));
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

static void
Constraint_Rotation(const std::shared_ptr<RuntimeNode>& src,
                    const std::shared_ptr<RuntimeNode>& dst,
                    float weight)
{
  auto delta = DirectX::XMQuaternionMultiply(
    DirectX::XMLoadFloat4(&src->Transform.Rotation),
    DirectX::XMQuaternionInverse(
      DirectX::XMLoadFloat4(&src->Node->InitialTransform.Rotation)));

  DirectX::XMStoreFloat4(
    &dst->Transform.Rotation,
    DirectX::XMQuaternionSlerp(
      DirectX::XMLoadFloat4(&dst->Node->InitialTransform.Rotation),
      DirectX::XMQuaternionMultiply(
        delta, DirectX::XMLoadFloat4(&dst->Node->InitialTransform.Rotation)),
      weight));
}

static DirectX::XMVECTOR
mul3(DirectX::XMVECTOR q0, DirectX::XMVECTOR q1, DirectX::XMVECTOR q2)
{
  return DirectX::XMQuaternionMultiply(DirectX::XMQuaternionMultiply(q0, q1),
                                       q2);
}

static DirectX::XMVECTOR
mul4(DirectX::XMVECTOR q0,
     DirectX::XMVECTOR q1,
     DirectX::XMVECTOR q2,
     DirectX::XMVECTOR q3)
{
  return DirectX::XMQuaternionMultiply(mul3(q0, q1, q2), q3);
}

static void
Constraint_Roll(const std::shared_ptr<RuntimeNode>& src,
                const std::shared_ptr<RuntimeNode>& dst,
                float weight,
                DirectX::XMVECTOR axis)
{
  auto deltaSrcQuat = DirectX::XMQuaternionMultiply(
    DirectX::XMLoadFloat4(&src->Transform.Rotation),
    DirectX::XMQuaternionInverse(
      DirectX::XMLoadFloat4(&src->Node->InitialTransform.Rotation)));
  auto deltaSrcQuatInParent =
    mul3(DirectX::XMQuaternionInverse(
           DirectX::XMLoadFloat4(&src->Node->InitialTransform.Rotation)),
         deltaSrcQuat,
         DirectX::XMLoadFloat4(&src->Node->InitialTransform.Rotation));
  auto deltaSrcQuatInDst =
    mul3(DirectX::XMLoadFloat4(&dst->Node->InitialTransform.Rotation),
         deltaSrcQuatInParent,
         DirectX::XMQuaternionInverse(
           DirectX::XMLoadFloat4(&dst->Node->InitialTransform.Rotation)));
  auto toVec = DirectX::XMQuaternionMultiply(axis, deltaSrcQuatInDst);
  auto fromToQuat = dmath::rotate_from_to(axis, toVec);

  DirectX::XMStoreFloat4(
    &dst->Transform.Rotation,
    DirectX::XMQuaternionSlerp(
      DirectX::XMLoadFloat4(&dst->Node->InitialTransform.Rotation),
      DirectX::XMQuaternionMultiply(DirectX::XMQuaternionInverse(fromToQuat),
                                    deltaSrcQuatInDst),
      weight));
}

static void
Constraint_Aim(const std::shared_ptr<RuntimeNode>& src,
               const std::shared_ptr<RuntimeNode>& dst,
               float weight,
               const DirectX::XMVECTOR axis)
{
  auto dstParentWorldQuat = dst->ParentWorldRotation();
  auto fromVec = DirectX::XMVector3Rotate(
    axis,
    DirectX::XMQuaternionMultiply(
      DirectX::XMLoadFloat4(&dst->Node->InitialTransform.Rotation),
      dstParentWorldQuat));
  auto toVec = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(
    DirectX::XMLoadFloat3(&src->WorldTransform.Translation),
    DirectX::XMLoadFloat3(&dst->WorldTransform.Translation)));
  auto fromToQuat = dmath::rotate_from_to(fromVec, toVec);

  DirectX::XMStoreFloat4(
    &dst->Transform.Rotation,
    DirectX::XMQuaternionSlerp(
      DirectX::XMLoadFloat4(&dst->Node->InitialTransform.Rotation),
      mul4(DirectX::XMLoadFloat4(&dst->Node->InitialTransform.Rotation),
           dstParentWorldQuat,
           fromToQuat,
           DirectX::XMQuaternionInverse(dstParentWorldQuat)),
      weight));
}

void
RuntimeScene::NodeConstraintProcess(const NodeConstraint& constraint,
                                    const std::shared_ptr<RuntimeNode>& dst)
{
  auto src = constraint.Source.lock();
  if (!src) {
    return;
  }

  switch (constraint.Type) {
    case NodeConstraintTypes::Rotation:
      Constraint_Rotation(src, dst, constraint.Weight);
      break;

    case NodeConstraintTypes::Roll:
      Constraint_Roll(
        src, dst, constraint.Weight, GetRollVector(constraint.RollAxis));
      break;

    case NodeConstraintTypes::Aim:
      Constraint_Aim(
        src, dst, constraint.Weight, GetAxisVector(constraint.AimAxis));
      break;
  }
}

} // namespace
