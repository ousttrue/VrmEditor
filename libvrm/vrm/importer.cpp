#include "importer.h"
#include "gltfroot.h"
#include "node.h"
#include <DirectXMath.h>
#include <gltfjson.h>
#include <gltfjson/gltf_typing_vrm0.h>
#include <gltfjson/gltf_typing_vrm1.h>
#include <gltfjson/json_tree_parser.h>

namespace libvrm {

static DirectX::XMFLOAT4
RotateY180(const DirectX::XMFLOAT4& src)
{
  auto r = DirectX::XMLoadFloat4(&src);
  auto y180 = DirectX::XMQuaternionRotationMatrix(
    DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(180.0f)));
  DirectX::XMFLOAT4 dst;
  DirectX::XMStoreFloat4(&dst, DirectX::XMQuaternionMultiply(y180, r));
  auto a = dst;
  return dst;
}

static std::expected<std::shared_ptr<Node>, std::string>
ParseNode(const std::shared_ptr<GltfRoot>& scene,
          int i,
          const gltfjson::Node& node)
{
  auto ptr = std::make_shared<Node>(node.Name());

  if (node.Matrix.size() == 16) {
    // matrix
    auto& m = node.Matrix;
    auto local = DirectX::XMFLOAT4X4{
      m[0],  m[1],  m[2],  m[3],  //
      m[4],  m[5],  m[6],  m[7],  //
      m[8],  m[9],  m[10], m[11], //
      m[12], m[13], m[14], m[15], //
    };
    ptr->SetLocalInitialMatrix(DirectX::XMLoadFloat4x4(&local));
  } else {
    // T
    if (node.Translation.size() == 3) {
      auto& t = node.Translation;
      ptr->InitialTransform.Translation = { t[0], t[1], t[2] };
    }
    // R
    if (node.Rotation.size() == 4) {
      auto& r = node.Rotation;
      ptr->InitialTransform.Rotation = { r[0], r[1], r[2], r[3] };
    }
    // S
    if (node.Scale.size() == 3) {
      auto& s = node.Scale;
      ptr->InitialScale = { s[0], s[1], s[2] };
    }
  }

  return ptr;
}

static std::expected<bool, std::string>
ParseVrm0(const std::shared_ptr<GltfRoot>& scene)
{
  auto VRM = scene->m_gltf->GetExtension<gltfjson::vrm0::VRM>();
  if (!VRM) {
    return std::unexpected{ "no extensions.VRM" };
  }

  if (auto humanoid = VRM->Humanoid()) {
    // bone & node
    for (auto humanBone : humanoid->HumanBones) {
      if (auto node = humanBone.Node()) {
        auto index = *node;
        auto name = humanBone.Bone();
        // std::cout << name << ": " << index << std::endl;
        if (auto bone =
              HumanBoneFromName(gltfjson::from_u8(name), VrmVersion::_0_x)) {
          scene->m_nodes[index]->Humanoid = *bone;
        }
      }
    }
  }

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

  if (auto secondaryAnimation = VRM->SecondaryAnimation()) {
    for (auto colliderGroup : secondaryAnimation->ColliderGroups) {
      auto group = std::make_shared<SpringColliderGroup>();
      auto node_index = colliderGroup.Node();
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

  for (auto& root : scene->m_roots) {
    root->InitialTransform.Rotation =
      RotateY180(root->InitialTransform.Rotation);
  }

  return true;
}

static std::expected<bool, std::string>
ParseVrm1(const std::shared_ptr<GltfRoot>& scene)
{
  auto extensions = scene->m_gltf->Extensions();
  if (!extensions) {
    return std::unexpected{ "no extensions" };
  }

  auto VRMC_vrm = extensions->Get(u8"VRMC_vrm");
  if (!VRMC_vrm) {
    return std::unexpected{ "no extensions.VRMC_vrm" };
  }

  if (auto humanoid = VRMC_vrm->Get(u8"humanoid")) {
    if (auto humanBones = humanoid->Get(u8"humanBones")) {
      if (auto object = humanBones->Object()) {
        for (auto& kv : *object) {
          auto name = kv.first;
          if (auto bone =
                HumanBoneFromName(gltfjson::from_u8(name), VrmVersion::_1_0)) {
            auto index = (uint32_t)*kv.second->Get(u8"node")->Ptr<float>();
            scene->m_nodes[index]->Humanoid = *bone;
          } else {
            std::cout << gltfjson::from_u8(name) << std::endl;
          }
        }
      }
    }
  }

  // if (has(extensions, "VRMC_springBone")) {
  //   auto& VRMC_springBone = extensions.at("VRMC_springBone");
  //   if (has(VRMC_springBone, "springs")) {
  //     auto& springs = VRMC_springBone.at("springs");
  //     for (auto& spring : springs) {
  //       auto springBone = std::make_shared<vrm::SpringBone>();
  //       std::shared_ptr<Node> head;
  //       for (auto& joint : spring.at("joints")) {
  //         int node_index = joint.at("node");
  //         auto tail = scene->m_nodes[node_index];
  //         if (head) {
  //           float stiffness = joint.at("stiffness");
  //           float dragForce = joint.at("dragForce");
  //           float radius = joint.at("hitRadius");
  //           springBone->AddJoint(head,
  //                                tail,
  //                                tail->InitialTransform.Translation,
  //                                stiffness,
  //                                dragForce,
  //                                radius);
  //         }
  //         head = tail;
  //       }
  //       scene->m_springBones.push_back(springBone);
  //     }
  //   }
  //   if (has(VRMC_springBone, "colliders")) {
  //     auto& colliders = VRMC_springBone.at("colliders");
  //     for (auto& collider : colliders) {
  //       auto ptr = std::make_shared<vrm::SpringCollider>();
  //       uint32_t node_index = collider.at("node");
  //       ptr->Node = scene->m_nodes[node_index];
  //       auto& shape = collider.at("shape");
  //       if (has(shape, "sphere")) {
  //         auto& sphere = shape.at("sphere");
  //         ptr->Type = vrm::SpringColliderShapeType::Sphere;
  //         ptr->Radius = sphere.value("radius", 0.0f);
  //         ptr->Offset = sphere.value("offset", DirectX::XMFLOAT3{ 0, 0, 0
  //         });
  //       } else if (has(shape, "capsule")) {
  //         auto& capsule = shape.at("capsule");
  //         ptr->Type = vrm::SpringColliderShapeType::Capsule;
  //         ptr->Radius = capsule.value("radius", 0.0f);
  //         ptr->Offset = capsule.value("offset", DirectX::XMFLOAT3{ 0, 0, 0
  //         }); ptr->Tail = capsule.value("tail", DirectX::XMFLOAT3{ 0, 0, 0
  //         });
  //       } else {
  //         assert(false);
  //       }
  //       scene->m_springColliders.push_back(ptr);
  //     }
  //   }
  //   if (has(VRMC_springBone, "colliderGroups")) {
  //     auto& colliderGroups = VRMC_springBone.at("colliderGroups");
  //     for (auto& colliderGroup : colliderGroups) {
  //       auto ptr = std::make_shared<vrm::SpringColliderGroup>();
  //       for (uint32_t collider_index : colliderGroup.at("colliders")) {
  //         ptr->Colliders.push_back(scene->m_springColliders[collider_index]);
  //       }
  //       scene->m_springColliderGroups.push_back(ptr);
  //     }
  //   }
  // }
  //
  // auto& nodes = scene->m_gltf->Json.at("nodes");
  // for (size_t i = 0; i < nodes.size(); ++i) {
  //   auto& node = nodes[i];
  //   auto ptr = scene->m_nodes[i];
  //   if (has(node, "extensions")) {
  //     auto& extensions = node.at("extensions");
  //     if (has(extensions, "VRMC_node_constraint")) {
  //       auto& VRMC_node_constraint = extensions.at("VRMC_node_constraint");
  //       if (has(VRMC_node_constraint, "constraint")) {
  //         auto& constraint = VRMC_node_constraint.at("constraint");
  //         // roll
  //         auto weight = constraint.value("weight", 1.0f);
  //         static DirectX::XMFLOAT4 s_constraint_color{ 1, 0.6f, 1, 1 };
  //         if (has(constraint, "roll")) {
  //           auto& roll = constraint.at("roll");
  //           int source_index = roll.at("source");
  //           ptr->Constraint = vrm::NodeConstraint{
  //             .Type = vrm::NodeConstraintTypes::Roll,
  //             .Source = scene->m_nodes[source_index],
  //             .Weight = weight,
  //           };
  //           std::string_view axis = roll.at("rollAxis");
  //           ptr->Constraint->RollAxis =
  //             vrm::NodeConstraintRollAxisFromName(axis);
  //           ptr->ShapeColor = s_constraint_color;
  //         }
  //         // aim
  //         if (has(constraint, "aim")) {
  //           auto& aim = constraint.at("aim");
  //           int source_index = aim.at("source");
  //           ptr->Constraint = vrm::NodeConstraint{
  //             .Type = vrm::NodeConstraintTypes::Aim,
  //             .Source = scene->m_nodes[source_index],
  //             .Weight = weight,
  //           };
  //           std::string_view axis = aim.at("aimAxis");
  //           ptr->Constraint->AimAxis =
  //           vrm::NodeConstraintAimAxisFromName(axis); ptr->ShapeColor =
  //           s_constraint_color;
  //         }
  //         // rotation
  //         if (has(constraint, "rotation")) {
  //           auto& rotation = constraint.at("rotation");
  //           int source_index = rotation.at("source");
  //           ptr->Constraint = vrm::NodeConstraint{
  //             .Type = vrm::NodeConstraintTypes::Rotation,
  //             .Source = scene->m_nodes[source_index],
  //             .Weight = weight,
  //           };
  //           ptr->ShapeColor = s_constraint_color;
  //         }
  //       }
  //     }
  //   }
  // }

  return true;
}

static std::expected<bool, std::string>
Parse(const std::shared_ptr<GltfRoot>& scene)
{
  scene->m_title = "glTF";

  // if (has(scene->m_gltf->Json, "extensionsRequired")) {
  //   for (auto& ex : scene->m_gltf->Json.at("extensionsRequired")) {
  //     if (ex == "KHR_draco_mesh_compression") {
  //       return std::unexpected{ "KHR_draco_mesh_compression" };
  //     }
  //     if (ex == "KHR_mesh_quantization") {
  //       return std::unexpected{ "KHR_mesh_quantization" };
  //     }
  //   }
  // }

  if (auto extensions = scene->m_gltf->Extensions()) {
    if (auto VRM = extensions->Get(u8"VRM")) {
      scene->m_type = ModelType::Vrm0;
      scene->m_title = "vrm-0.x";
      scene->m_vrm0Materials = VRM->Get(u8"materialProperties");
    }
    if (extensions->Get(u8"VRMC_vrm")) {
      scene->m_type = ModelType::Vrm1;
      scene->m_title = "vrm-1.0";
    }
  }

  {
    auto& nodes = scene->m_gltf->Nodes;
    for (int i = 0; i < nodes.size(); ++i) {
      if (auto node = ParseNode(scene, i, nodes[i])) {
        scene->m_nodes.push_back(*node);
      } else {
        return std::unexpected{ node.error() };
      }
    }
    for (int i = 0; i < nodes.size(); ++i) {
      auto node = nodes[i];
      for (auto child : node.Children) {
        Node::AddChild(scene->m_nodes[i], scene->m_nodes[child]);
      }
    }
  }
  {
    auto _scene = scene->m_gltf->Scenes[0];
    for (auto node : _scene.Nodes) {
      scene->m_roots.push_back(scene->m_nodes[node]);
    }
  }

  // calc world
  auto enter = [](const std::shared_ptr<Node>& node) {
    node->CalcWorldInitialMatrix();
    // node->CalcInitialMatrix();
    return true;
  };
  scene->Traverse(enter, {});

  if (scene->m_type == ModelType::Vrm0) {
    if (auto vrm0 = ParseVrm0(scene)) {
    } else {
      return std::unexpected{ vrm0.error() };
    }
  }

  if (scene->m_type == ModelType::Vrm1) {
    if (auto vrm1 = ParseVrm1(scene)) {
    } else {
      return std::unexpected{ vrm1.error() };
    }
  }

  scene->InitializeNodes();

  return true;
}

static std::expected<bool, std::string>
Load(const std::shared_ptr<GltfRoot>& scene,
     std::span<const uint8_t> json_chunk,
     std::span<const uint8_t> bin_chunk,
     const std::shared_ptr<gltfjson::Directory>& dir)
{
  gltfjson::tree::Parser parser(json_chunk);
  if (auto result = parser.ParseExpected()) {
    gltfjson::tree::Parser parser(json_chunk);
    scene->m_gltf = std::make_shared<gltfjson::Root>(parser.Parse());
    scene->m_bin = { dir, bin_chunk };
    if (!scene->m_bin.Dir) {
      scene->m_bin.Dir = std::make_shared<gltfjson::Directory>();
    }
    return Parse(scene);
  } else {
    auto error = result.error();
    std::string msg{ (const char*)error.data(),
                     (const char*)error.data() + error.size() };
    return std::unexpected{ msg };
  }
}

std::expected<bool, std::string>
LoadBytes(const std::shared_ptr<GltfRoot>& scene,
          std::span<const uint8_t> bytes,
          const std::shared_ptr<gltfjson::Directory>& dir)
{
  scene->m_bytes.assign(bytes.begin(), bytes.end());
  if (auto glb = gltfjson::Glb::Parse(scene->m_bytes)) {
    // as glb
    return Load(scene, glb->JsonChunk, glb->BinChunk, dir);
  }

  // try gltf
  return Load(scene, scene->m_bytes, {}, dir);
}

std::expected<std::shared_ptr<GltfRoot>, std::string>
LoadPath(const std::filesystem::path& path)
{
  if (auto bytes = gltfjson::ReadAllBytes(path)) {
    auto ptr = std::make_shared<GltfRoot>();
    if (auto load = LoadBytes(
          ptr,
          *bytes,
          std::make_shared<gltfjson::Directory>(path.parent_path()))) {
      return ptr;
    } else {
      return std::unexpected(load.error());
    }
  } else {
    return std::unexpected{ bytes.error() };
  }
}

} // namespace
