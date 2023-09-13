#include "runtime_scene.h"
#include "animation.h"
#include "dmath.h"
#include "expression.h"
#include "gizmo.h"
#include "humanoid/humanskeleton.h"
#include "runtime_node.h"
#include "spring_collision.h"
#include <boneskin/node_state.h>
#include <gltfjson.h>
#include <gltfjson/gltf_typing_vrm0.h>
#include <gltfjson/gltf_typing_vrm1.h>
#include <gltfjson/json_tree_exporter.h>

namespace libvrm {

static std::shared_ptr<Node>
FindNode(const std::shared_ptr<libvrm::GltfRoot>& root, uint32_t meshTarget)
{
  for (int i = 0; i < root->m_gltf->Nodes.size(); ++i) {
    auto node = root->m_gltf->Nodes[i];
    if (auto mesh = node.MeshId()) {
      if (*mesh == meshTarget) {
        return root->m_nodes[i];
      }
    }
  }
  // PLOG_WARN << "node not found";
  return {};
}

static void
ParseVrm0(RuntimeScene* scene, const gltfjson::vrm0::VRM& VRM)
{
  // meta
  // specVersion
  // exporterVersion
  // firstPerson

  if (auto expression = VRM.BlendShapeMaster()) {
    scene->m_expressions = std::make_shared<Expressions>();
    for (auto g : expression->BlendShapeGroups) {
      auto preset = libvrm::ExpressionPresetFromVrm0String(g.PresetString());
      if (!preset) {
        preset = libvrm::ExpressionPresetFromVrm0String(g.NameString());
      }
      Expression* expression = nullptr;
      if (preset) {
        expression = &scene->m_expressions->Preset(*preset);
      } else {
        scene->m_expressions->CustomExpressions.push_back({});
        expression = &scene->m_expressions->CustomExpressions.back();
      }
      expression->isBinary = g.IsBinaryOrFalse();
      for (auto bind : g.MorphBinds) {
        expression->morphBinds.push_back({});
        auto& back = expression->morphBinds.back();
        back.mesh = *bind.MeshId();
        back.index = *bind.MorphIndexId();
        back.weight = *bind.Weight() *= 0.01f;
        back.Node = FindNode(scene->m_base, back.mesh);
      }
    }
  }

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
AddExpression(RuntimeScene* scene,
              const std::u8string& presetName,
              const std::u8string& name,
              std::optional<gltfjson::vrm1::Expression> src)
{
  if (src) {
    Expression* expression = nullptr;
    if (auto preset = libvrm::ExpressionPresetFromVrm1String(presetName)) {
      expression = &scene->m_expressions->Preset(*preset);
    } else {
      // custom
      scene->m_expressions->CustomExpressions.push_back({});
      expression = &scene->m_expressions->CustomExpressions.back();
    }
    expression->isBinary = src->IsBinaryOrFalse();
    for (auto morph : src->MorphTargetBinds) {
      expression->morphBinds.push_back({});
      auto& back = expression->morphBinds.back();
      back.Node = scene->m_base->m_nodes[*morph.NodeId()];
      back.index = *morph.IndexId();
      back.weight = *morph.Weight();
    }
  }
}

static void
ParseConstraint(RuntimeScene* scene)
{
  auto& nodes = scene->m_base->m_gltf->Nodes;
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
          };
          if (auto p = roll->Weight()) {
            ptr->Constraint->Weight = *p;
          }
          auto axis = roll->RollAxisString();
          ptr->Constraint->RollAxis =
            NodeConstraintRollAxisFromName(gltfjson::from_u8(axis));
          ptr->Base->ShapeColor = s_constraint_color;
        } else if (auto aim = constraint->Aim()) {
          // aim
          auto source_index = aim->SourceId();
          ptr->Constraint = NodeConstraint{
            .Type = NodeConstraintTypes::Aim,
            .Source = scene->m_nodes[*source_index],
          };
          if (auto p = aim->Weight()) {
            ptr->Constraint->Weight = *p;
          }
          auto axis = aim->AimAxisString();
          ptr->Constraint->AimAxis =
            NodeConstraintAimAxisFromName(gltfjson::from_u8(axis));
          ptr->Base->ShapeColor = s_constraint_color;
        } else if (auto rotation = constraint->Rotation()) {
          // rotation
          auto source_index = rotation->SourceId();
          ptr->Constraint = NodeConstraint{
            .Type = NodeConstraintTypes::Rotation,
            .Source = scene->m_nodes[*source_index],
          };
          if (auto p = rotation->Weight()) {
            ptr->Constraint->Weight = *p;
          }
          ptr->Base->ShapeColor = s_constraint_color;
        } else {
          assert(false);
        }
      }
    }
  }
}

static void
ParseVrm1(RuntimeScene* scene, const gltfjson::vrm1::VRMC_vrm& VRMC_vrm)
{
  if (auto VRMC_vrm =
        scene->m_base->m_gltf->GetExtension<gltfjson::vrm1::VRMC_vrm>()) {
    if (auto expressions = VRMC_vrm->Expressions()) {
      scene->m_expressions = std::make_shared<Expressions>();
      if (auto preset = expressions->Preset()) {
        AddExpression(scene, u8"happy", u8"happy", preset->Happy());
        AddExpression(scene, u8"angry", u8"angry", preset->Angry());
        AddExpression(scene, u8"sad", u8"sad", preset->Sad());
        AddExpression(scene, u8"relaxed", u8"relaxed", preset->Relaxed());
        AddExpression(scene, u8"surprised", u8"surprised", preset->Surprised());
        AddExpression(scene, u8"aa", u8"aa", preset->Aa());
        AddExpression(scene, u8"ih", u8"ih", preset->Ih());
        AddExpression(scene, u8"ou", u8"ou", preset->Ou());
        AddExpression(scene, u8"ee", u8"ee", preset->Ee());
        AddExpression(scene, u8"oh", u8"oh", preset->Oh());
        AddExpression(scene, u8"blink", u8"blink", preset->Blink());
        AddExpression(scene, u8"blinkLeft", u8"blinkLeft", preset->BlinkLeft());
        AddExpression(
          scene, u8"blinkRight", u8"blinkRight", preset->BlinkRight());
        AddExpression(scene, u8"lookUp", u8"lookUp", preset->LookUp());
        AddExpression(scene, u8"lookDown", u8"lookDown", preset->LookDown());
        AddExpression(scene, u8"lookLeft", u8"lookLeft", preset->LookLeft());
        AddExpression(scene, u8"lookRight", u8"lookRight", preset->LookRight());
        AddExpression(scene, u8"neutral", u8"neutral", preset->Neutral());
      }
    }
  }

  if (auto VRMC_springBone =
        scene->m_base->m_gltf
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
    //       // ptr->Offset = capsule.value("offset", DirectX::XMFLOAT3{ 0, 0,
    //       0
    //       // }); ptr->Tail = capsule.value("tail", DirectX::XMFLOAT3{ 0, 0,
    //       0
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
    //   //
    //   ptr->Colliders.push_back(scene->m_springColliders[collider_index]);
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
                               tail->Base->InitialTransform.Translation,
                               stiffness,
                               dragForce,
                               radius);
        }
        head = tail;
      }
      scene->m_springBones.push_back(springBone);
    }
  }
}

static std::shared_ptr<Animation>
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
                                u8"-translation",
                              sampler.InterpolationEnum());
        } else {
          // return std::unexpected{ values.error() };
          return {};
        }
      } else if (path == u8"rotation") {
        if (auto values =
              bin.GetAccessorBytes<DirectX::XMFLOAT4>(root, output_index)) {
          ptr->AddRotation(node_index,
                           *times,
                           *values,
                           root.Nodes[node_index].NameString() + u8"-rotation",
                           sampler.InterpolationEnum());
        } else {
          // return std::unexpected{ values.error() };
          return {};
        }
      } else if (path == u8"scale") {
        if (auto values =
              bin.GetAccessorBytes<DirectX::XMFLOAT3>(root, output_index)) {
          ptr->AddScale(node_index,
                        *times,
                        *values,
                        root.Nodes[node_index].NameString() + u8"-scale",
                        sampler.InterpolationEnum());
        } else {
          // return std::unexpected{ values.error() };
          return {};
        }
      } else if (path == u8"weights") {
        if (auto values = bin.GetAccessorBytes<float>(root, output_index)) {
          auto node = root.Nodes[node_index];
          if (auto mesh = node.MeshId()) {
            if (values->size() !=
                root.Meshes[*mesh].Primitives[0].Targets.size() *
                  times->size()) {
              // return std::unexpected{ "animation-weights: size not match" };
              return {};
            }
            ptr->AddWeights(node_index,
                            *times,
                            *values,
                            node.NameString() + u8"-weights",
                            sampler.InterpolationEnum());
          } else {
            // return std::unexpected{ "animation-weights: no node.mesh" };
            return {};
          }
        } else {
          // return std::unexpected{ values.error() };
          return {};
        }
      } else {
        // return std::unexpected{ "animation path is not implemented: " };
        return {};
      }
    } else {
      // return std::unexpected{ times.error() };
      return {};
    }
  }

  return ptr;
}

RuntimeScene::RuntimeScene(const std::shared_ptr<GltfRoot>& table)
  : m_base(table)
  , m_timeline(new Timeline)
{
  Reset();
}

RuntimeScene::~RuntimeScene()
{
  //
  auto a = 0;
}

std::shared_ptr<RuntimeScene>
RuntimeScene::Load(const std::shared_ptr<GltfRoot>& base)
{
  assert(base);
  auto ptr = std::make_shared<RuntimeScene>(base);

  if (base->m_gltf) {
    if (auto VRMC_vrm =
          base->m_gltf->GetExtension<gltfjson::vrm1::VRMC_vrm>()) {
      ParseVrm1(ptr.get(), *VRMC_vrm);
    } else if (auto VRM = base->m_gltf->GetExtension<gltfjson::vrm0::VRM>()) {
      ParseVrm0(ptr.get(), *VRM);
    }

    ParseConstraint(ptr.get());

    for (int i = 0; i < base->m_gltf->Animations.size(); ++i) {
      if (auto animation = ParseAnimation(*base->m_gltf, base->m_bin, i)) {
        ptr->m_animations.push_back(animation);
      }
    }

    if (auto VRMC_vrm_animation =
          base->m_gltf->GetExtension<gltfjson::vrm1::VRMC_vrm_animation>()) {
      if (auto VRMC_vrm_pose =
            VRMC_vrm_animation->GetExtension<gltfjson::vrm1::VRMC_vrm_pose>()) {

        if (auto humanoid = VRMC_vrm_pose->Humanoid()) {
          for (auto kv : *humanoid) {
            if (kv.first == u8"translation") {
              if (auto node = ptr->GetBoneNode(HumanBones::hips)) {
                auto v = ToVec3(kv.second);
                node->Transform.Translation = v;
              }
            }
            if (kv.first == u8"rotations") {
              if (auto rotations =
                    std::dynamic_pointer_cast<gltfjson::tree::ObjectNode>(
                      kv.second)) {
                for (auto [key, value] : rotations->Value) {
                  if (auto bone = HumanBoneFromName(gltfjson::from_u8(key),
                                                    VrmVersion::_1_0)) {
                    if (auto node = ptr->GetBoneNode(*bone)) {
                      DirectX::XMFLOAT4 q = ToVec4(value);
                      node->Transform.Rotation = q;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  return ptr;
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

  if (m_base) {
    std::unordered_map<std::shared_ptr<Node>, std::shared_ptr<RuntimeNode>>
      nodeMap;

    // COPY hierarchy
    for (auto& node : m_base->m_nodes) {
      auto runtime = std::make_shared<RuntimeNode>(node);
      nodeMap.insert({ node, runtime });
      m_nodes.push_back(runtime);
    }

    for (auto& node : m_base->m_nodes) {
      auto runtime = nodeMap[node];
      if (auto parent = node->Parent.lock()) {
        auto runtimeParent = nodeMap[parent];
        RuntimeNode::AddChild(runtimeParent, runtime);
      } else {
        m_roots.push_back(runtime);
      }
    }
  }
}

std::shared_ptr<RuntimeNode>
RuntimeScene::GetBoneNode(HumanBones bone)
{
  for (auto& node : m_nodes) {
    if (auto humanoid = node->Base->Humanoid) {
      if (*humanoid == bone) {
        return node;
      }
    }
  }
  return {};
}

std::shared_ptr<RuntimeSpringJoint>
RuntimeScene::GetOrCreateRuntimeJoint(const std::shared_ptr<SpringJoint>& joint)
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
RuntimeScene::GetOrCreateRuntimeSpringCollision(
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
RuntimeScene::UpdateNodeStates(std::span<boneskin::NodeState> nodestates)
{
  if (!m_base->m_gltf) {
    return;
  }
  // base->m_sceneUpdated.push_back([=](const auto&)
  {
    for (auto& root : m_roots) {
      root->CalcWorldMatrix(true);
    }
    // Reset();
  };

  // glTF morph animation
  for (int i = 0; i < m_nodes.size(); ++i) {
    auto found = m_moprhWeigts.find(i);
    if (found != m_moprhWeigts.end()) {
      auto& item = nodestates[i];
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
    auto nodeToIndex = [nodes = m_base->m_nodes, expressions = m_expressions](
                         const std::shared_ptr<Node>& node) {
      for (uint32_t i = 0; i < nodes.size(); ++i) {
        if (node == nodes[i]) {
          return i;
        }
      }
      return (uint32_t)-1;
    };
    for (auto& [k, v] : m_expressions->EvalMorphTargetMap(nodeToIndex)) {
      auto& item = nodestates[k.NodeIndex];
      item.MorphMap[k.MorphIndex] = v;
    }
  }

  for (uint32_t i = 0; i < nodestates.size(); ++i) {
    // model matrix
    DirectX::XMStoreFloat4x4(&nodestates[i].Matrix, m_nodes[i]->WorldMatrix());
  }

  UpdateHumanPose();
}

std::span<const DirectX::XMFLOAT4X4>
RuntimeScene::ShapeMatrices()
{
  m_shapeMatrices.clear();
  for (auto& node : m_nodes) {
    m_shapeMatrices.push_back({});
    auto shape = DirectX::XMLoadFloat4x4(&node->Base->ShapeMatrix);
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

  auto collision = GetOrCreateRuntimeSpringCollision(spring);
  for (auto& joint : spring->Joints) {
    collision->Clear();
    auto runtime = GetOrCreateRuntimeJoint(joint);
    runtime->Update(delta, collision.get());
  }
}

const DirectX::XMFLOAT4 MAGENTA = { 1, 0, 1, 1 };
const DirectX::XMFLOAT4 YELLOW = { 1, 1, 0, 1 };
const DirectX::XMFLOAT4 RED = { 1, 0.5f, 0, 1 };
void
RuntimeScene::SpringDrawGizmo(const std::shared_ptr<SpringBone>& solver,
                              IGizmoDrawer* gizmo)
{
  for (auto& joint : solver->Joints) {
    auto runtime = GetOrCreateRuntimeJoint(joint);
    auto color = MAGENTA;
    if (joint == m_springJointSelected) {
      color = RED;
    } else if (solver == m_springBoneSelected) {
      color = YELLOW;
    }
    runtime->DrawGizmo(gizmo, color);
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
    if (auto humanoid = node->Base->Humanoid) {
      m_humanBoneMap.push_back(*humanoid);
      if (m_humanBoneMap.back() == HumanBones::hips) {
        // delta move
        DirectX::XMStoreFloat3(
          &m_pose.RootPosition,
          DirectX::XMVectorSubtract(
            DirectX::XMLoadFloat3(&node->WorldTransform.Translation),
            DirectX::XMLoadFloat3(
              &node->Base->WorldInitialTransform.Translation)));
      }

      // retarget
      auto normalized = mult4(
        DirectX::XMQuaternionInverse(
          DirectX::XMLoadFloat4(&node->Base->WorldInitialTransform.Rotation)),
        DirectX::XMLoadFloat4(&node->Transform.Rotation),
        DirectX::XMQuaternionInverse(
          DirectX::XMLoadFloat4(&node->Base->InitialTransform.Rotation)),
        DirectX::XMLoadFloat4(&node->Base->WorldInitialTransform.Rotation));

      if (*humanoid != HumanBones::hips &&
          DirectX::XMQuaternionIsIdentity(normalized)) {
        // skip
        m_humanBoneMap.pop_back();
      } else {
        m_rotations.push_back({});
        DirectX::XMStoreFloat4(&m_rotations.back(), normalized);
      }
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
      auto init = node->Base;
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
      DirectX::XMLoadFloat4(&src->Base->InitialTransform.Rotation)));

  DirectX::XMStoreFloat4(
    &dst->Transform.Rotation,
    DirectX::XMQuaternionSlerp(
      DirectX::XMLoadFloat4(&dst->Base->InitialTransform.Rotation),
      DirectX::XMQuaternionMultiply(
        delta, DirectX::XMLoadFloat4(&dst->Base->InitialTransform.Rotation)),
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
      DirectX::XMLoadFloat4(&src->Base->InitialTransform.Rotation)));
  auto deltaSrcQuatInParent =
    mul3(DirectX::XMQuaternionInverse(
           DirectX::XMLoadFloat4(&src->Base->InitialTransform.Rotation)),
         deltaSrcQuat,
         DirectX::XMLoadFloat4(&src->Base->InitialTransform.Rotation));
  auto deltaSrcQuatInDst =
    mul3(DirectX::XMLoadFloat4(&dst->Base->InitialTransform.Rotation),
         deltaSrcQuatInParent,
         DirectX::XMQuaternionInverse(
           DirectX::XMLoadFloat4(&dst->Base->InitialTransform.Rotation)));
  auto toVec = DirectX::XMQuaternionMultiply(axis, deltaSrcQuatInDst);
  auto fromToQuat = dmath::rotate_from_to(axis, toVec);

  DirectX::XMStoreFloat4(
    &dst->Transform.Rotation,
    DirectX::XMQuaternionSlerp(
      DirectX::XMLoadFloat4(&dst->Base->InitialTransform.Rotation),
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
      DirectX::XMLoadFloat4(&dst->Base->InitialTransform.Rotation),
      dstParentWorldQuat));
  auto toVec = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(
    DirectX::XMLoadFloat3(&src->WorldTransform.Translation),
    DirectX::XMLoadFloat3(&dst->WorldTransform.Translation)));
  auto fromToQuat = dmath::rotate_from_to(fromVec, toVec);

  DirectX::XMStoreFloat4(
    &dst->Transform.Rotation,
    DirectX::XMQuaternionSlerp(
      DirectX::XMLoadFloat4(&dst->Base->InitialTransform.Rotation),
      mul4(DirectX::XMLoadFloat4(&dst->Base->InitialTransform.Rotation),
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

std::string
RuntimeScene::CopyVrmPoseText()
{
  if (auto skeleton = m_base->GetHumanSkeleton()) {
    auto gltf = gltfjson::gltf::CreateEmpty();

    // T-pose
    for (size_t i = 0; i < skeleton->Bones.size(); ++i) {
      auto& bone = skeleton->Bones[i];
      auto name = HumanBoneToName(bone.HumanBone);
      auto node = gltfjson::gltf::CreateNode();
      node.m_json->SetProperty(u8"name", std::u8string((const char8_t*)name));
      if (i == 0) {
        // root
        node.m_json->SetProperty(u8"translation", bone.WorldPosition);
        node.m_json->SetProperty(u8"rotation", bone.WorldRotation);
        gltf.Scenes[0].Nodes.m_json->Add((float)0);
      } else {
        gltf.Nodes[bone.ParentIndex].Children.m_json->Add((float)i);
        auto& parent = skeleton->Bones[bone.ParentIndex];
        auto [pos, rot] = bone.ToLocal(parent);
        node.m_json->SetProperty(u8"translation", pos);
        node.m_json->SetProperty(u8"rotation", rot);
      }
      gltf.Nodes.push_back(node);
    }

    // VRMC_vrm_animation
    auto VRMC_vrm_animation =
      gltf.GetOrCreateExtension<gltfjson::vrm1::VRMC_vrm_animation>();
    auto vrmaHumanoid = VRMC_vrm_animation.m_json->SetProperty(
      u8"humanoid", gltfjson::tree::ObjectValue());
    {
      auto vrmaHumanBones = vrmaHumanoid->SetProperty(
        u8"humanBones", gltfjson::tree::ObjectValue());

      for (int i = 0; i < skeleton->Bones.size(); ++i) {
        auto& bone = skeleton->Bones[i];
        auto name = HumanBoneToName(bone.HumanBone);
        auto vrmaHumanBone = vrmaHumanBones->SetProperty(
          (const char8_t*)name, gltfjson::tree::ObjectValue());
        vrmaHumanBone->SetProperty(u8"node", (float)i);
      }
    }

    auto pose = UpdateHumanPose();
    if (pose.Bones.size()) {
      // {
      //   "translation": [x, y, z],
      //   "hips": [x, y, z, w],
      //   "spine": [x, y, z, w],
      //   "chest": [x, y, z, w],
      //   ...
      // }
      auto vrmaFrame =
        vrmaHumanoid->SetProperty(u8"frame", gltfjson::tree::ObjectValue());

      auto [node, _] = m_base->GetBoneNode(HumanBones::hips);
      auto base = node->WorldInitialTransform.Translation;
      std::array<float, 3> pos = {
        pose.RootPosition.x + base.x,
        pose.RootPosition.y + base.y,
        pose.RootPosition.z + base.z,
      };

      vrmaFrame->SetProperty(u8"translation", pos);
      for (int i = 0; i < pose.Bones.size(); ++i) {
        auto name = HumanBoneToName(pose.Bones[i]);
        vrmaFrame->SetProperty(
          (const char8_t*)name,
          *((const std::array<float, 4>*)&pose.Rotations[i]));
      }
    }

    std::stringstream ss;
    gltfjson::StringSink write = [&ss](std::string_view src) mutable {
      ss.write(src.data(), src.size());
    };
    gltfjson::tree::Exporter exporter{ write };
    exporter.Export(gltf.m_json);

    return ss.str();
  }

  return {};
}

std::shared_ptr<RuntimeNode>
RuntimeScene::GetSelectedNode() const
{
  for (auto& node : m_nodes) {
    if (node && node->Base == m_base->m_selected) {
      return node;
    }
  }
  return {};
}

void
RuntimeScene::SelectNode(const std::shared_ptr<libvrm::RuntimeNode>& node)
{
  m_base->SelectNode(node ? node->Base : std::shared_ptr<libvrm::Node>());
  for (auto& spring : m_springBones) {
    for (auto& joint : spring->Joints) {
      if (joint->Head == node) {
        m_springBoneSelected = spring;
        m_springJointSelected = joint;
        return;
      }
    }
  }
}

bool
RuntimeScene::IsSelected(const std::shared_ptr<libvrm::RuntimeNode>& node) const
{
  return m_base->IsSelected(node->Base);
}

void
RuntimeScene::SelectJoint(const std::shared_ptr<libvrm::SpringJoint>& joint)
{
  m_springJointSelected = joint;
  m_base->m_selected = joint->Head->Base;
}

} // namespace
