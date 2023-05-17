#include "vrm/importer.h"
#include "vrm/animation.h"
#include "vrm/image.h"
#include "vrm/material.h"
#include "vrm/mesh.h"
#include "vrm/node.h"
#include "vrm/scene.h"
#include "vrm/skin.h"
#include "vrm/texture.h"
#include <gltfjson/deserialize.h>
#include <gltfjson/directory.h>
#include <gltfjson/glb.h>
#include <gltfjson/gltf.h>
#include <gltfjson/json.h>

namespace libvrm {
namespace gltf {

static std::expected<std::shared_ptr<gltf::Image>, std::string>
ParseImage(const std::shared_ptr<Scene>& scene,
           int i,
           const gltfjson::format::Image& image)
{
  std::span<const uint8_t> bytes;
  if (auto bufferView = image.BufferView) {
    if (auto buffer_view =
          scene->m_bin.GetBufferViewBytes(scene->m_gltf, *bufferView)) {
      bytes = *buffer_view;
    } else {
      return std::unexpected{ buffer_view.error() };
    }
  } else if (image.Uri.size()) {
    if (auto buffer_view = scene->m_bin.Dir->GetBuffer(image.Uri)) {
      bytes = *buffer_view;
    } else {
      return std::unexpected{ buffer_view.error() };
    }
  } else {
    return std::unexpected{ "not bufferView nor uri" };
  }
  auto name = image.Name;
  auto ptr = std::make_shared<gltf::Image>(name);
  if (!ptr->Load(bytes)) {
    return std::unexpected{ name };
  }
  return ptr;
}

static std::expected<std::shared_ptr<gltf::Texture>, std::string>
ParseTexture(const std::shared_ptr<Scene>& scene,
             int i,
             const gltfjson::format::Texture& texture)
{
  auto name = texture.Name;
  auto ptr = std::make_shared<gltf::Texture>();
  ptr->Source = scene->m_images[*texture.Source];
  if (auto sampler = texture.Sampler) {
    ptr->Sampler = scene->m_samplers[*sampler];
  }
  return ptr;
}

static std::expected<std::shared_ptr<gltf::Material>, std::string>
ParseMaterial(const std::shared_ptr<Scene>& scene,
              int i,
              const gltfjson::format::Material& material)
{
  auto ptr = std::make_shared<gltf::Material>(material.Name);

  // if (has(material, "extensions")) {
  //   auto& extensions = material.at("extensions");
  //   if (has(extensions, "KHR_materials_unlit")) {
  //     ptr->Type = MaterialTypes::UnLit;
  //   }
  // }
  if (auto pbr = material.PbrMetallicRoughness) {
    if (auto texture = pbr->BaseColorTexture) {
      int texture_index = *texture->Index;
      ptr->Pbr.BaseColorTexture = scene->m_textures[texture_index];
      ptr->Pbr.BaseColorTexture->ColorSpace = ColorSpace::sRGB;
    }
    ptr->Pbr.BaseColorFactor = pbr->BaseColorFactor;
    ptr->Pbr.MetallicFactor = pbr->MetallicFactor;
    ptr->Pbr.RoughnessFactor = pbr->RoughnessFactor;
    if (auto texture = pbr->MetallicRoughnessTexture) {
      int texture_index = *texture->Index;
      ptr->Pbr.MetallicRoughnessTexture = scene->m_textures[texture_index];
      ptr->Pbr.MetallicRoughnessTexture->ColorSpace = ColorSpace::Linear;
    }
  }
  if (auto texture = material.NormalTexture) {
    int texture_index = *texture->Index;
    ptr->NormalTexture = scene->m_textures[texture_index];
    ptr->NormalTexture->ColorSpace = ColorSpace::Linear;
    ptr->NormalTextureScale = texture->Scale;
  }
  if (auto texture = material.OcclusionTexture) {
    int texture_index = *texture->Index;
    ptr->OcclusionTexture = scene->m_textures[texture_index];
    ptr->OcclusionTexture->ColorSpace = ColorSpace::Linear;
    ptr->OcclusionTextureStrength = texture->Strength;
  }
  if (auto texture = material.EmissiveTexture) {
    int texture_index = *texture->Index;
    ptr->EmissiveTexture = scene->m_textures[texture_index];
    ptr->EmissiveTexture->ColorSpace = ColorSpace::sRGB;
  }
  ptr->EmissiveFactor = material.EmissiveFactor;
  ptr->AlphaMode = material.AlphaMode;
  ptr->AlphaCutoff = material.AlphaCutoff;
  ptr->DoubleSided = material.DoubleSided;
  return ptr;
}

static std::expected<bool, std::string>
AddIndices(const std::shared_ptr<Scene>& scene,
           int vertex_offset,
           gltf::Mesh* mesh,
           const gltfjson::format::MeshPrimitive& prim,
           const std::shared_ptr<gltf::Material>& material)
{
  if (prim.Indices) {
    int accessor_index = *prim.Indices;
    auto accessor = scene->m_gltf.Accessors[accessor_index];
    switch (accessor.ComponentType) {
      case gltfjson::format::ComponentTypes::UNSIGNED_BYTE: {
        if (auto span =
              scene->m_bin.GetAccessorBytes<uint8_t>(scene->m_gltf, accessor_index)) {
          mesh->addSubmesh(vertex_offset, *span, material);
          return true;
        } else {
          return std::unexpected{ span.error() };
        }
      } break;
      case gltfjson::format::ComponentTypes::UNSIGNED_SHORT: {
        if (auto span =
              scene->m_bin.GetAccessorBytes<uint16_t>(scene->m_gltf, accessor_index)) {
          mesh->addSubmesh(vertex_offset, *span, material);
          return true;
        } else {
          return std::unexpected{ span.error() };
        }
      } break;
      case gltfjson::format::ComponentTypes::UNSIGNED_INT: {
        if (auto span =
              scene->m_bin.GetAccessorBytes<uint32_t>(scene->m_gltf, accessor_index)) {
          mesh->addSubmesh(vertex_offset, *span, material);
          return true;
        } else {
          return std::unexpected{ span.error() };
        }
      } break;
      default:
        return std::unexpected{ "invalid index type" };
    }
  } else {
    std::vector<uint32_t> indices;
    auto vertex_count = mesh->m_vertices.size();
    indices.reserve(vertex_count);
    for (int i = 0; i < vertex_count; ++i) {
      indices.push_back(i);
    }
    mesh->addSubmesh<uint32_t>(vertex_offset, indices, material);
    return true;
  }
}

static std::expected<std::shared_ptr<gltf::Mesh>, std::string>
ParseMesh(const std::shared_ptr<Scene>& scene,
          int i,
          const gltfjson::format::Mesh& mesh)
{
  auto ptr = std::make_shared<gltf::Mesh>();
  ptr->Name = mesh.Name;
  gltfjson::format::MeshPrimitiveAttributes lastAtributes = {};
  for (auto& prim : mesh.Primitives) {
    std::shared_ptr<gltf::Material> material;
    if (prim.Material) {
      material = scene->m_materials[*prim.Material];
    } else {
      // default material
      material = std::make_shared<gltf::Material>("default");
    }

    if (prim.Attributes == lastAtributes) {
      // for vrm shared vertex buffer
      if (auto expected = AddIndices(scene, 0, ptr.get(), prim, material)) {
        // OK
      } else {
        return std::unexpected{ expected.error() };
      }
    } else {
      // extend vertex buffer
      std::span<const DirectX::XMFLOAT3> positions;
      if (auto accessor = scene->m_bin.GetAccessorBytes<DirectX::XMFLOAT3>(
            scene->m_gltf, *prim.Attributes.POSITION)) {
        positions = *accessor;
      } else {
        return std::unexpected{ accessor.error() };
      }
      std::vector<DirectX::XMFLOAT3> copy;
      if (scene->m_type == ModelType::Vrm0) {
        copy.reserve(positions.size());
        for (auto& p : positions) {
          copy.push_back({ -p.x, p.y, -p.z });
        }
        positions = copy;
      }
      auto offset = ptr->addPosition(positions);

      if (prim.Attributes.NORMAL) {
        if (auto accessor = scene->m_bin.GetAccessorBytes<DirectX::XMFLOAT3>(
              scene->m_gltf, *prim.Attributes.NORMAL)) {
          ptr->setNormal(offset, *accessor);
        } else {
          return std::unexpected{ accessor.error() };
        }
      }

      if (prim.Attributes.TEXCOORD_0) {
        if (auto accessor = scene->m_bin.GetAccessorBytes<DirectX::XMFLOAT2>(
              scene->m_gltf, *prim.Attributes.TEXCOORD_0)) {
          ptr->setUv(offset, *accessor);
        } else {
          return std::unexpected{ accessor.error() };
        }
      }

      if (prim.Attributes.JOINTS_0 && prim.Attributes.WEIGHTS_0) {
        // skinning
        int joint_accessor = *prim.Attributes.JOINTS_0;
        auto item_size = scene->m_gltf.Accessors[joint_accessor].Stride();
        switch (item_size) {
          case 4:
            if (auto accessor =
                  scene->m_bin.GetAccessorBytes<byte4>(scene->m_gltf, joint_accessor)) {
              if (auto accessor_w = scene->m_bin.GetAccessorBytes<DirectX::XMFLOAT4>(
                    scene->m_gltf, *prim.Attributes.WEIGHTS_0)) {
                ptr->setBoneSkinning(offset, *accessor, *accessor_w);
              } else {
                return std::unexpected{ accessor_w.error() };
              }
            } else {
              return std::unexpected{ accessor.error() };
            }
            break;

          case 8:
            if (auto accessor = scene->m_bin.GetAccessorBytes<ushort4>(
                  scene->m_gltf, joint_accessor)) {
              if (auto accessor_w = scene->m_bin.GetAccessorBytes<DirectX::XMFLOAT4>(
                    scene->m_gltf, *prim.Attributes.WEIGHTS_0)) {
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
          auto& target = targets.at(i);
          auto morph = ptr->getOrCreateMorphTarget(i);
          // std::cout << target << std::endl;
          std::span<const DirectX::XMFLOAT3> positions;
          if (auto accessor = scene->m_bin.GetAccessorBytes<DirectX::XMFLOAT3>(
                scene->m_gltf, *target.POSITION)) {
            positions = *accessor;
          } else {
            return std::unexpected{ accessor.error() };
          }
          std::vector<DirectX::XMFLOAT3> copy;
          if (scene->m_type == ModelType::Vrm0) {
            copy.reserve(positions.size());
            for (auto& p : positions) {
              copy.push_back({ -p.x, p.y, -p.z });
            }
            positions = copy;
          }
          /*auto morphOffset =*/morph->addPosition(positions);
        }
      }

      // extend indices and add vertex offset
      if (auto expected =
            AddIndices(scene, offset, ptr.get(), prim, material)) {
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

    lastAtributes = prim.Attributes;
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

static std::expected<std::shared_ptr<gltf::Skin>, std::string>
ParseSkin(const std::shared_ptr<Scene>& scene,
          int i,
          const gltfjson::format::Skin& skin)
{
  auto ptr = std::make_shared<gltf::Skin>();
  ptr->Name = skin.Name;
  for (auto& joint : skin.Joints) {
    ptr->Joints.push_back(joint);
  }

  std::span<const DirectX::XMFLOAT4X4> matrices;
  if (auto accessor = scene->m_bin.GetAccessorBytes<DirectX::XMFLOAT4X4>(
        scene->m_gltf, *skin.InverseBindMatrices)) {
    matrices = *accessor;
  } else {
    return std::unexpected{ accessor.error() };
  }
  std::vector<DirectX::XMFLOAT4X4> copy;
  if (scene->m_type == ModelType::Vrm0) {
    copy.reserve(matrices.size());
    for (auto& m : matrices) {
      copy.push_back(m);
      copy.back()._41 = -m._41;
      copy.back()._43 = -m._43;
    }
    matrices = copy;
  }
  ptr->BindMatrices.assign(matrices.begin(), matrices.end());

  assert(ptr->Joints.size() == ptr->BindMatrices.size());

  ptr->Root = skin.Skeleton;
  return ptr;
}

static std::expected<std::shared_ptr<gltf::Node>, std::string>
ParseNode(const std::shared_ptr<Scene>& scene,
          int i,
          const gltfjson::format::Node& node)
{
  auto ptr = std::make_shared<gltf::Node>(node.Name);

  if (auto matrix = node.Matrix) {
    // matrix
    auto m = *matrix;
    auto local = DirectX::XMFLOAT4X4{
      m[0],  m[1],  m[2],  m[3],  //
      m[4],  m[5],  m[6],  m[7],  //
      m[8],  m[9],  m[10], m[11], //
      m[12], m[13], m[14], m[15], //
    };
    ptr->SetLocalInitialMatrix(DirectX::XMLoadFloat4x4(&local));
  } else {
    // T
    if (auto t = node.Translation) {
      ptr->InitialTransform.Translation = { (*t)[0], (*t)[1], (*t)[2] };
    }
    if (scene->m_type == ModelType::Vrm0) {
      // rotate: Y180
      auto t = ptr->InitialTransform.Translation;
      ptr->InitialTransform.Translation = { -t.x, t.y, -t.z };
    }
    // R
    if (auto r = node.Rotation) {
      ptr->InitialTransform.Rotation = { (*r)[0], (*r)[1], (*r)[2], (*r)[3] };
    }
    // S
    if (auto s = node.Scale) {
      ptr->InitialScale = { (*s)[0], (*s)[1], (*s)[2] };
    }
  }

  if (node.Mesh) {
    auto mesh_index = *node.Mesh;
    auto mesh = scene->m_meshes[mesh_index];
    ptr->Mesh = mesh;

    if (node.Skin) {
      int skin_index = *node.Skin;
      auto skin = scene->m_skins[skin_index];
      ptr->Skin = skin;
    }
  }

  return ptr;
}

static std::expected<std::shared_ptr<gltf::Animation>, std::string>
ParseAnimation(const std::shared_ptr<Scene>& scene,
               int i,
               const gltfjson::format::Animation& animation)
{
  auto ptr = std::make_shared<gltf::Animation>(animation.Name);

  // samplers
  auto& samplers = animation.Samplers;

  // channels
  auto& channels = animation.Channels;
  for (auto& channel : channels) {
    int sampler_index = *channel.Sampler;
    auto sampler = samplers[sampler_index];

    auto target = channel.Target;
    int node_index = *target.Node;
    auto path = target.Path;

    // time
    int input_index = *sampler.Input;
    if (auto times = scene->m_bin.GetAccessorBytes<float>(scene->m_gltf, input_index)) {
      int output_index = *sampler.Output;
      if (path == gltfjson::format::PathTypes::Translation) {
        if (auto values = scene->m_bin.GetAccessorBytes<DirectX::XMFLOAT3>(
              scene->m_gltf, output_index)) {
          ptr->AddTranslation(node_index,
                              *times,
                              *values,
                              scene->m_nodes[node_index]->Name +
                                "-translation");
        } else {
          return std::unexpected{ values.error() };
        }
      } else if (path == gltfjson::format::PathTypes::Rotation) {
        if (auto values = scene->m_bin.GetAccessorBytes<DirectX::XMFLOAT4>(
              scene->m_gltf, output_index)) {
          ptr->AddRotation(node_index,
                           *times,
                           *values,
                           scene->m_nodes[node_index]->Name + "-rotation");
        } else {
          return std::unexpected{ values.error() };
        }
      } else if (path == gltfjson::format::PathTypes::Scale) {
        if (auto values = scene->m_bin.GetAccessorBytes<DirectX::XMFLOAT3>(
              scene->m_gltf, output_index)) {
          ptr->AddScale(node_index,
                        *times,
                        *values,
                        scene->m_nodes[node_index]->Name + "-scale");
        } else {
          return std::unexpected{ values.error() };
        }
      } else if (path == gltfjson::format::PathTypes::Weights) {
        if (auto values =
              scene->m_bin.GetAccessorBytes<float>(scene->m_gltf, output_index)) {
          auto node = scene->m_nodes[node_index];
          if (node->Mesh) {
            if (values->size() !=
                node->Mesh->m_morphTargets.size() * times->size()) {
              return std::unexpected{ "animation-weights: size not match" };
            }
            ptr->AddWeights(
              node_index, *times, *values, node->Name + "-weights");
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

static std::expected<bool, std::string>
ParseVrm0(const std::shared_ptr<Scene>& scene)
{
  // if (!has(scene->m_gltf.Json, "extensions")) {
  //   return std::unexpected{ "no extensions" };
  // }
  // auto& extensions = scene->m_gltf.Json.at("extensions");
  //
  // if (!has(extensions, "VRM")) {
  //   return std::unexpected{ "no extensions.VRM" };
  // }
  // auto VRM = extensions.at("VRM");
  //
  // if (has(VRM, "humanoid")) {
  //   auto& humanoid = VRM.at("humanoid");
  //   if (has(humanoid, "humanBones")) {
  //     auto& humanBones = humanoid.at("humanBones");
  //     // bone & node
  //     for (auto& humanBone : humanBones) {
  //       int index = humanBone.at("node");
  //       std::string_view name = humanBone.at("bone");
  //       // std::cout << name << ": " << index << std::endl;
  //       if (auto bone = vrm::HumanBoneFromName(name, vrm::VrmVersion::_0_x))
  //       {
  //         scene->m_nodes[index]->Humanoid = *bone;
  //       }
  //     }
  //   }
  // }
  //
  // // meta
  // // specVersion
  // // exporterVersion
  //
  // // firstPerson
  // // materialProperties
  // if (has(VRM, "materialProperties")) {
  //   auto& props = VRM.at("materialProperties");
  //   for (int i = 0; i < props.size(); ++i) {
  //     if (props[i].at("shader") == "VRM/MToon") {
  //       // TODO
  //       scene->m_materials[i]->Type = MaterialTypes::MToon0;
  //     }
  //   }
  // }
  //
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
  //
  // if (has(VRM, "secondaryAnimation")) {
  //   auto& secondaryAnimation = VRM.at("secondaryAnimation");
  //   if (has(secondaryAnimation, "colliderGroups")) {
  //     auto& colliderGroups = secondaryAnimation.at("colliderGroups");
  //     for (auto& colliderGroup : colliderGroups) {
  //       auto group = std::make_shared<vrm::SpringColliderGroup>();
  //       uint32_t node_index = colliderGroup.at("node");
  //       auto colliderNode = scene->m_nodes[node_index];
  //       if (has(colliderGroup, "colliders")) {
  //         for (auto& collider : colliderGroup.at("colliders")) {
  //           auto item = std::make_shared<vrm::SpringCollider>();
  //           auto& offset = collider.at("offset");
  //           float x = offset.at("x");
  //           float y = offset.at("y");
  //           float z = offset.at("z");
  //           // vrm0: springbone collider offset is
  //           // UnityCoordinate(LeftHanded)
  //           item->Offset = { -x, y, z };
  //           item->Radius = collider.at("radius");
  //           item->Node = colliderNode;
  //           scene->m_springColliders.push_back(item);
  //           group->Colliders.push_back(item);
  //         }
  //       }
  //       scene->m_springColliderGroups.push_back(group);
  //     }
  //   }
  //   if (has(secondaryAnimation, "boneGroups")) {
  //     auto& boneGroups = secondaryAnimation.at("boneGroups");
  //     for (auto& boneGroup : boneGroups) {
  //       float stiffness = boneGroup.at("stiffiness");
  //       float dragForce = boneGroup.at("dragForce");
  //       float radius = boneGroup.at("hitRadius");
  //       std::vector<std::shared_ptr<vrm::SpringColliderGroup>>
  //       colliderGroups; if (has(boneGroup, "colliderGroups")) {
  //         for (uint32_t colliderGroup_index : boneGroup.at("colliderGroups"))
  //         {
  //           auto colliderGroup =
  //             scene->m_springColliderGroups[colliderGroup_index];
  //           colliderGroups.push_back(colliderGroup);
  //         }
  //       }
  //       for (auto& bone : boneGroup.at("bones")) {
  //         auto spring = std::make_shared<vrm::SpringBone>();
  //         spring->AddJointRecursive(
  //           scene->m_nodes[bone], dragForce, stiffness, radius);
  //         scene->m_springBones.push_back(spring);
  //         for (auto& g : colliderGroups) {
  //           spring->AddColliderGroup(g);
  //         }
  //       }
  //     }
  //   }
  // }
  //
  return true;
}

static std::expected<bool, std::string>
ParseVrm1(const std::shared_ptr<Scene>& scene)
{
  // if (!has(scene->m_gltf.Json, "extensions")) {
  //   return std::unexpected{ "no extensions" };
  // }
  // auto& extensions = scene->m_gltf.Json.at("extensions");
  //
  // if (!has(extensions, "VRMC_vrm")) {
  //   return std::unexpected{ "no extensions.VRMC_vrm" };
  // }
  //
  // auto VRMC_vrm = extensions.at("VRMC_vrm");
  // if (has(VRMC_vrm, "humanoid")) {
  //   auto& humanoid = VRMC_vrm.at("humanoid");
  //   if (has(humanoid, "humanBones")) {
  //     auto& humanBones = humanoid.at("humanBones");
  //     for (auto& kv : humanBones.items()) {
  //       if (auto bone =
  //             vrm::HumanBoneFromName(kv.key(), vrm::VrmVersion::_1_0)) {
  //         scene->m_nodes[kv.value().at("node")]->Humanoid = *bone;
  //       } else {
  //         std::cout << kv.key() << std::endl;
  //       }
  //     }
  //   }
  // }
  //
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
  //         ptr->Offset = sphere.value("offset", DirectX::XMFLOAT3{ 0, 0, 0 });
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
  // auto& nodes = scene->m_gltf.Json.at("nodes");
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
Parse(const std::shared_ptr<Scene>& scene)
{
  scene->m_title = "glTF";

  // if (has(scene->m_gltf.Json, "extensionsRequired")) {
  //   for (auto& ex : scene->m_gltf.Json.at("extensionsRequired")) {
  //     if (ex == "KHR_draco_mesh_compression") {
  //       return std::unexpected{ "KHR_draco_mesh_compression" };
  //     }
  //     if (ex == "KHR_mesh_quantization") {
  //       return std::unexpected{ "KHR_mesh_quantization" };
  //     }
  //   }
  // }

  // if (has(scene->m_gltf.Json, "extensions")) {
  //   auto& extensions = scene->m_gltf.Json.at("extensions");
  //   if (has(extensions, "VRM")) {
  //     auto VRM = extensions.at("VRM");
  //     // TODO: meta
  //     scene->m_type = ModelType::Vrm0;
  //     scene->m_title = "vrm-0.x";
  //   }
  //   if (has(extensions, "VRMC_vrm")) {
  //     scene->m_type = ModelType::Vrm1;
  //     scene->m_title = "vrm-1.0";
  //   }
  // }

  {
    auto& samplers = scene->m_gltf.Samplers;
    for (int i = 0; i < samplers.Size(); ++i) {
      // if (auto sampler = ParseTextureSampler(i, samplers[i])) {
      //   scene->m_samplers.push_back(*sampler);
      // } else {
      //   return std::unexpected{ sampler.error() };
      // }
    }
  }

  {
    auto& images = scene->m_gltf.Images;
    for (int i = 0; i < images.Size(); ++i) {
      if (auto image = ParseImage(scene, i, images[i])) {
        scene->m_images.push_back(*image);
      } else {
        return std::unexpected{ image.error() };
      }
    }
  }

  {
    auto& textures = scene->m_gltf.Textures;
    for (int i = 0; i < textures.Size(); ++i) {
      if (auto texture = ParseTexture(scene, i, textures[i])) {
        scene->m_textures.push_back(*texture);
      } else {
        return std::unexpected{ texture.error() };
      }
    }
  }

  {
    auto& materials = scene->m_gltf.Materials;
    for (int i = 0; i < materials.Size(); ++i) {
      if (auto material = ParseMaterial(scene, i, materials[i])) {
        scene->m_materials.push_back(*material);
      } else {
        return std::unexpected{ material.error() };
      }
    }
  }

  {
    auto& meshes = scene->m_gltf.Meshes;
    for (int i = 0; i < meshes.Size(); ++i) {
      if (auto mesh = ParseMesh(scene, i, meshes[i])) {
        scene->m_meshes.push_back(*mesh);
      } else {
        return std::unexpected{ mesh.error() };
      }
    }
  }

  {
    auto skins = scene->m_gltf.Skins;
    for (int i = 0; i < skins.Size(); ++i) {
      if (auto skin = ParseSkin(scene, i, skins[i])) {
        scene->m_skins.push_back(*skin);
      } else {
        return std::unexpected{ skin.error() };
      }
    }
  }

  {
    auto nodes = scene->m_gltf.Nodes;
    for (int i = 0; i < nodes.Size(); ++i) {
      if (auto node = ParseNode(scene, i, nodes[i])) {
        scene->m_nodes.push_back(*node);
      } else {
        return std::unexpected{ node.error() };
      }
    }
    for (int i = 0; i < nodes.Size(); ++i) {
      auto& node = nodes[i];
      for (auto child : node.Children) {
        gltf::Node::AddChild(scene->m_nodes[i], scene->m_nodes[child]);
      }
    }
  }
  {
    auto _scene = scene->m_gltf.Scenes[0];
    for (auto& node : _scene.Nodes) {
      scene->m_roots.push_back(scene->m_nodes[node]);
    }
  }

  // calc world
  auto enter = [](const std::shared_ptr<gltf::Node>& node) {
    node->CalcWorldInitialMatrix();
    // node->CalcInitialMatrix();
    return true;
  };
  scene->Traverse(enter, {});

  {
    auto& animations = scene->m_gltf.Animations;
    for (int i = 0; i < animations.Size(); ++i) {
      if (auto animation = ParseAnimation(scene, i, animations[i])) {
        scene->m_animations.push_back(*animation);
      } else {
        return std::unexpected{ animation.error() };
      }
    }
  }

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
Load(const std::shared_ptr<Scene>& scene,
     std::span<const uint8_t> json_chunk,
     std::span<const uint8_t> bin_chunk,
     const std::shared_ptr<gltfjson::Directory>& dir)
{
  gltfjson::Parser parser(json_chunk);
  if (auto result = parser.Parse()) {
    gltfjson::Deserialize(parser, scene->m_gltf);
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
LoadBytes(const std::shared_ptr<Scene>& scene,
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

std::expected<std::shared_ptr<Scene>, std::string>
LoadPath(const std::filesystem::path& path)
{
  if (auto bytes = gltfjson::ReadAllBytes(path)) {
    auto ptr = std::make_shared<Scene>();
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
}
}
