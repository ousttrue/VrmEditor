#include "runtime_scene.h"
#include "../deformed_mesh.h"
#include "../skin.h"
#include "animation.h"
#include "runtime_node.h"
#include "spring_collision.h"
#include <gltfjson.h>

namespace runtimescene {

static std::expected<bool, std::string>
AddIndices(const gltfjson::Root& root,
           const gltfjson::Bin& bin,
           int vertex_offset,
           BaseMesh* mesh,
           const gltfjson::MeshPrimitive& prim)
{
  if (auto indices = prim.Indices()) {
    auto accessor_index = (uint32_t)*indices;
    auto accessor = root.Accessors[accessor_index];
    switch ((gltfjson::ComponentTypes)*accessor.ComponentType()) {
      case gltfjson::ComponentTypes::UNSIGNED_BYTE: {
        if (auto span = bin.GetAccessorBytes<uint8_t>(root, accessor_index)) {
          mesh->addSubmesh(vertex_offset, *span, prim.Material());
          return true;
        } else {
          return std::unexpected{ span.error() };
        }
      } break;
      case gltfjson::ComponentTypes::UNSIGNED_SHORT: {
        if (auto span = bin.GetAccessorBytes<uint16_t>(root, accessor_index)) {
          mesh->addSubmesh(vertex_offset, *span, prim.Material());
          return true;
        } else {
          return std::unexpected{ span.error() };
        }
      } break;
      case gltfjson::ComponentTypes::UNSIGNED_INT: {
        if (auto span = bin.GetAccessorBytes<uint32_t>(root, accessor_index)) {
          mesh->addSubmesh(vertex_offset, *span, prim.Material());
          return true;
        } else {
          return std::unexpected{ span.error() };
        }
      } break;
      default:
        return std::unexpected{ "invalid index type" };
    }
  } else {
    std::vector<uint32_t> indexList;
    auto vertex_count = mesh->m_vertices.size();
    indexList.reserve(vertex_count);
    for (int i = 0; i < vertex_count; ++i) {
      indexList.push_back(i);
    }
    mesh->addSubmesh<uint32_t>(vertex_offset, indexList, prim.Material());
    return true;
  }
}

static std::string
u8_to_str(const std::u8string& src)
{
  return { (const char*)src.data(), (const char*)src.data() + src.size() };
}

static std::expected<std::shared_ptr<BaseMesh>, std::string>
ParseMesh(const gltfjson::Root& root, const gltfjson::Bin& bin, int meshIndex)
{
  auto mesh = root.Meshes[meshIndex];
  auto ptr = std::make_shared<BaseMesh>();
  ptr->Name = mesh.Name();
  std::optional<gltfjson::MeshPrimitiveAttributes> lastAtributes;

  for (auto prim : mesh.Primitives) {
    if (prim.Attributes() == lastAtributes) {
      // for vrm shared vertex buffer
      if (auto expected = AddIndices(root, bin, 0, ptr.get(), prim)) {
        // OK
      } else {
        return std::unexpected{ expected.error() };
      }
    } else {
      // extend vertex buffer
      std::span<const DirectX::XMFLOAT3> positions;
      if (auto accessor = bin.GetAccessorBytes<DirectX::XMFLOAT3>(
            root, *prim.Attributes()->POSITION())) {
        positions = *accessor;
      } else {
        return std::unexpected{ accessor.error() };
      }
      // if (scene->m_type == ModelType::Vrm0) {
      // std::vector<DirectX::XMFLOAT3> copy;
      //   copy.reserve(positions.size());
      //   for (auto& p : positions) {
      //     copy.push_back({ -p.x, p.y, -p.z });
      //   }
      //   positions = copy;
      // }
      auto offset = ptr->addPosition(positions);

      if (auto normal = prim.Attributes()->NORMAL()) {
        if (auto accessor =
              bin.GetAccessorBytes<DirectX::XMFLOAT3>(root, *normal)) {
          ptr->setNormal(offset, *accessor);
        } else {
          return std::unexpected{ accessor.error() };
        }
      }

      if (auto tex0 = prim.Attributes()->TEXCOORD_0()) {
        if (auto accessor =
              bin.GetAccessorBytes<DirectX::XMFLOAT2>(root, *tex0)) {
          ptr->setUv(offset, *accessor);
        } else {
          return std::unexpected{ accessor.error() };
        }
      }

      auto joints0 = prim.Attributes()->JOINTS_0();
      auto weights0 = prim.Attributes()->WEIGHTS_0();
      if (joints0 && weights0) {
        // skinning
        int joint_accessor = *joints0;
        auto item_size = root.Accessors[joint_accessor].Stride();
        switch (item_size) {
          case 4:
            if (auto accessor =
                  bin.GetAccessorBytes<byte4>(root, joint_accessor)) {
              if (auto accessor_w =
                    bin.GetAccessorBytes<DirectX::XMFLOAT4>(root, *weights0)) {
                ptr->setBoneSkinning(offset, *accessor, *accessor_w);
              } else {
                return std::unexpected{ accessor_w.error() };
              }
            } else {
              return std::unexpected{ accessor.error() };
            }
            break;

          case 8:
            if (auto accessor =
                  bin.GetAccessorBytes<ushort4>(root, joint_accessor)) {
              if (auto accessor_w =
                    bin.GetAccessorBytes<DirectX::XMFLOAT4>(root, *weights0)) {
                ptr->setBoneSkinning(offset, *accessor, *accessor_w);
              } else {
                return std::unexpected{ accessor_w.error() };
              }
            } else {
              return std::unexpected{ accessor.error() };
            }
            break;

          default:
            // not implemented
            return std::unexpected{ "JOINTS_0 is not ushort4" };
        }
      }

      // extend morph target
      {
        auto& targets = prim.Targets;
        for (int i = 0; i < targets.size(); ++i) {
          auto target = targets[i];
          auto morph = ptr->getOrCreateMorphTarget(i);
          // std::cout << target << std::endl;
          std::span<const DirectX::XMFLOAT3> positions;
          if (auto accessor = bin.GetAccessorBytes<DirectX::XMFLOAT3>(
                root, *target.POSITION())) {
            positions = *accessor;
          } else {
            return std::unexpected{ accessor.error() };
          }
          // if (scene->m_type == ModelType::Vrm0) {
          //   std::vector<DirectX::XMFLOAT3> copy;
          //   copy.reserve(positions.size());
          //   for (auto& p : positions) {
          //     copy.push_back({ -p.x, p.y, -p.z });
          //   }
          //   positions = copy;
          // }
          /*auto morphOffset =*/morph->addPosition(positions);
        }
      }

      // extend indices and add vertex offset
      if (auto expected = AddIndices(root, bin, offset, ptr.get(), prim)) {
        // OK
      } else {
        return std::unexpected{ expected.error() };
      }
    }

    // find morph target name
    // primitive.extras.targetNames
    // if (has(prim, "extras")) {
    //   auto& extras = prim.at("extras");
    //   if (has(extras, "targetNames")) {
    //     auto& names = extras.at("targetNames");
    //     // std::cout << names << std::endl;
    //     for (int i = 0; i < names.size(); ++i) {
    //       ptr->getOrCreateMorphTarget(i)->Name = names[i];
    //     }
    //   }
    // }

    lastAtributes = *prim.Attributes();
  }

  // find morph target name
  // mesh.extras.targetNames
  // if (has(mesh, "extras")) {
  //   auto& extras = mesh.at("extras");
  //   if (has(extras, "targetNames")) {
  //     auto& names = extras.at("targetNames");
  //     // std::cout << names << std::endl;
  //     for (int i = 0; i < names.size(); ++i) {
  //       ptr->getOrCreateMorphTarget(i)->Name = names[i];
  //     }
  //   }
  // }

  return ptr;
}

static std::expected<std::shared_ptr<Skin>, std::string>
ParseSkin(const gltfjson::Root& root, const gltfjson::Bin& bin, int i)
{
  auto skin = root.Skins[i];
  auto ptr = std::make_shared<Skin>();
  ptr->Name = u8_to_str(skin.Name());
  for (auto joint : skin.Joints) {
    ptr->Joints.push_back(joint);
  }

  std::span<const DirectX::XMFLOAT4X4> matrices;
  if (auto accessor = bin.GetAccessorBytes<DirectX::XMFLOAT4X4>(
        root, *skin.InverseBindMatrices())) {
    matrices = *accessor;
  } else {
    return std::unexpected{ accessor.error() };
  }
  std::vector<DirectX::XMFLOAT4X4> copy;
  // if (scene->m_type == ModelType::Vrm0) {
  //   copy.reserve(matrices.size());
  //   for (auto& m : matrices) {
  //     copy.push_back(m);
  //     copy.back()._41 = -m._41;
  //     copy.back()._43 = -m._43;
  //   }
  //   matrices = copy;
  // }
  ptr->BindMatrices.assign(matrices.begin(), matrices.end());

  assert(ptr->Joints.size() == ptr->BindMatrices.size());

  ptr->Root = skin.Skeleton();
  return ptr;
}

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

RuntimeScene::RuntimeScene(const std::shared_ptr<libvrm::gltf::GltfRoot>& table)
  : m_table(table)
{
  Reset();

  if (m_table->m_gltf) {
    for (uint32_t i = 0; i < m_table->m_gltf->Meshes.size(); ++i) {
      auto baseMesh = ParseMesh(*m_table->m_gltf, m_table->m_bin, i);
      m_meshes.push_back(*baseMesh);
    }

    for (uint32_t i = 0; i < m_table->m_gltf->Skins.size(); ++i) {
      auto skin = ParseSkin(*m_table->m_gltf, m_table->m_bin, i);
      m_skins.push_back(*skin);
    }
  }
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

std::shared_ptr<DeformedMesh>
RuntimeScene::GetDeformedMesh(uint32_t mesh)
{
  auto found = m_meshMap.find(mesh);
  if (found != m_meshMap.end()) {
    return found->second;
  }

  auto runtime = std::make_shared<DeformedMesh>(m_meshes[mesh]);
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
  if (!m_table->m_gltf) {
    return {};
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
      auto morph_node = m_table->m_gltf->Nodes[k.NodeIndex];
      if (auto mesh = morph_node.Mesh()) {
        if (auto instance = GetDeformedMesh(*mesh)) {
          instance->Weights[k.MorphIndex] = v;
        }
      }
    }
  }

  // skinning
  for (uint32_t i = 0; i < m_table->m_gltf->Nodes.size(); ++i) {
    auto gltfNode = m_table->m_gltf->Nodes[i];
    if (auto mesh = gltfNode.Mesh()) {
      // auto mesh = m_table->m_meshes[*mesh_index];

      // mesh animation
      std::span<DirectX::XMFLOAT4X4> skinningMatrices;

      // skinning
      if (auto skinIndex = gltfNode.Skin()) {
        auto skin = m_skins[*skinIndex];
        skin->CurrentMatrices.resize(skin->BindMatrices.size());

        auto rootInverse = DirectX::XMMatrixIdentity();
        if (auto root_index = skin->Root) {
          rootInverse = DirectX::XMMatrixInverse(
            nullptr, GetRuntimeNode(m_table->m_nodes[i])->WorldMatrix());
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
      if (auto mesh = gltfNode.Mesh()) {
        if (auto instance = GetDeformedMesh(*mesh)) {
          instance->applyMorphTargetAndSkinning(*m_meshes[*mesh],
                                                skinningMatrices);
        }
      }
    }
  }

  m_drawables.clear();
  for (uint32_t i = 0; i < m_table->m_gltf->Nodes.size(); ++i) {
    auto gltfNode = m_table->m_gltf->Nodes[i];
    if (auto mesh = gltfNode.Mesh()) {
      m_drawables.push_back({
        .Mesh = *mesh,
      });
      DirectX::XMStoreFloat4x4(
        &m_drawables.back().Matrix,
        GetRuntimeNode(m_table->m_nodes[i])->WorldMatrix());
      // auto instance = GetDeformedMesh(*mesh);
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
RuntimeScene::NodeConstraintProcess(
  const libvrm::vrm::NodeConstraint& constraint,
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
