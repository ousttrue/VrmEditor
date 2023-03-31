#include "vrm/scene.h"
#include "vrm/animation.h"
#include "vrm/glb.h"
#include "vrm/material.h"
#include "vrm/mesh.h"
#include "vrm/node.h"
#include "vrm/skin.h"
#include "vrm/springbone.h"
#include "vrm/vrm0.h"
#include <DirectXMath.h>
#include <array>
#include <expected>
#include <format>
#include <fstream>
#include <iostream>
#include <list>
#include <optional>
#include <span>
#include <string>
#include <vector>

static DirectX::XMFLOAT4X4 IDENTITY{
    1, 0, 0, 0, //
    0, 1, 0, 0, //
    0, 0, 1, 0, //
    0, 0, 0, 1, //
};

// matrix
inline void to_json(nlohmann::json &j, const DirectX::XMFLOAT4X4 &m) {
  j = nlohmann::json{
      m._11, m._12, m._13, m._14, m._21, m._22, m._23, m._24,
      m._31, m._32, m._33, m._34, m._41, m._42, m._43, m._44,
  };
}
inline void from_json(const nlohmann::json &j, DirectX::XMFLOAT4X4 &m) {
  m._11 = j[0];
  m._12 = j[1];
  m._13 = j[2];
  m._14 = j[3];
  m._21 = j[4];
  m._22 = j[5];
  m._23 = j[6];
  m._24 = j[7];
  m._31 = j[8];
  m._32 = j[9];
  m._33 = j[10];
  m._34 = j[11];
  m._41 = j[12];
  m._42 = j[13];
  m._43 = j[14];
  m._44 = j[15];
}

Scene::Scene() { m_spring = std::make_shared<vrm::SpringSolver>(); }

std::expected<void, std::string>
Scene::Load(const std::filesystem::path &path) {
  auto bytes = ReadAllBytes(path);
  if (bytes.empty()) {
    return std::unexpected{std::format("fail to read: {}", path.string())};
  }

  if (auto glb = Glb::parse(bytes)) {
    // as glb
    return Load(path, glb->json, glb->bin);
  }

  // try gltf
  return Load(path, bytes, {});
}

std::expected<void, std::string>
Scene::Load(const std::filesystem::path &path,
            std::span<const uint8_t> json_chunk,
            std::span<const uint8_t> bin_chunk) {

  try {
    auto parsed = nlohmann::json::parse(json_chunk);
    auto dir = std::make_shared<Directory>(path.parent_path());
    m_gltf = {dir, parsed, bin_chunk};
  } catch (nlohmann::json::parse_error &e) {
    return std::unexpected{"json parse"};
  }

  if (has(m_gltf.json, "extensionsRequired")) {
    for (auto &ex : m_gltf.json.at("extensionsRequired")) {
      if (ex == "KHR_draco_mesh_compression") {
        return std::unexpected{"KHR_draco_mesh_compression"};
      }
      if (ex == "KHR_mesh_quantization") {
        return std::unexpected{"KHR_mesh_quantization"};
      }
    }
  }

  if (has(m_gltf.json, "extensions")) {
    auto &extensions = m_gltf.json.at("extensions");
    if (has(extensions, "VRM")) {
      auto VRM = extensions.at("VRM");
      m_vrm0 = std::make_shared<vrm0::Vrm>();
    }
  }

  auto &images = m_gltf.json["images"];
  for (int i = 0; i < images.size(); ++i) {
    auto &image = images[i];
    std::span<const uint8_t> bytes;
    if (has(image, "bufferView")) {
      bytes = m_gltf.buffer_view(image.at("bufferView"));
    } else if (has(image, "uri")) {
      bytes = m_gltf.m_dir->GetBuffer(image.at("uri"));
    } else {
      return std::unexpected{"not bufferView nor uri"};
    }
    auto name = image.value("name", std::format("image{}", i));
    auto ptr = std::make_shared<gltf::Image>(name);
    if (!ptr->load(bytes)) {
      return std::unexpected{name};
    }
    m_images.push_back(ptr);
  }

  auto &textures = m_gltf.json["textures"];

  auto &materials = m_gltf.json["materials"];
  for (int i = 0; i < materials.size(); ++i) {
    auto &material = materials[i];
    auto ptr = std::make_shared<gltf::Material>(
        material.value("name", std::format("material{}", i)));
    m_materials.push_back(ptr);
    if (has(material, "pbrMetallicRoughness")) {
      auto pbrMetallicRoughness = material.at("pbrMetallicRoughness");
      if (has(pbrMetallicRoughness, "baseColorTexture")) {
        auto &baseColorTexture = pbrMetallicRoughness.at("baseColorTexture");
        int texture_index = baseColorTexture.at("index");
        auto texture = textures.at(texture_index);
        int image_index = texture["source"];
        ptr->texture = m_images[image_index];
      }
    }
  }

  for (auto &mesh : m_gltf.json["meshes"]) {
    auto ptr = std::make_shared<gltf::Mesh>();
    m_meshes.push_back(ptr);

    nlohmann::json *lastAtributes = nullptr;
    for (auto &prim : mesh["primitives"]) {
      std::shared_ptr<gltf::Material> material;
      if (has(prim, "material")) {
        material = m_materials[prim.at("material")];
      } else {
        // default material
        material = std::make_shared<gltf::Material>("default");
      }

      nlohmann::json &attributes = prim.at("attributes");
      if (lastAtributes && attributes == *lastAtributes) {
        // for vrm shared vertex buffer
        AddIndices(0, ptr.get(), prim.at("indices"), material);
      } else {
        // extend vertex buffer
        auto positions =
            m_gltf.accessor<DirectX::XMFLOAT3>(attributes[VERTEX_POSITION]);
        std::vector<DirectX::XMFLOAT3> copy;
        if (m_vrm0) {
          copy.reserve(positions.size());
          for (auto &p : positions) {
            copy.push_back({-p.x, p.y, -p.z});
          }
          positions = copy;
        }
        auto offset = ptr->addPosition(positions);
        if (has(attributes, VERTEX_NORMAL)) {
          ptr->setNormal(offset, m_gltf.accessor<DirectX::XMFLOAT3>(
                                     attributes.at(VERTEX_NORMAL)));
        }
        if (has(attributes, VERTEX_UV)) {
          ptr->setUv(offset, m_gltf.accessor<DirectX::XMFLOAT2>(
                                 attributes.at(VERTEX_UV)));
        }

        if (has(attributes, VERTEX_JOINT) && has(attributes, VERTEX_WEIGHT)) {
          // skinning
          int joint_accessor = attributes.at(VERTEX_JOINT);
          switch (*item_size(m_gltf.json["accessors"][joint_accessor])) {
          case 8:
            ptr->setBoneSkinning(offset,
                                 m_gltf.accessor<ushort4>(joint_accessor),
                                 m_gltf.accessor<DirectX::XMFLOAT4>(
                                     attributes.at(VERTEX_WEIGHT)));
            break;

          default:
            // not implemented
            return std::unexpected{"JOINTS_0 is not ushort4"};
          }
        }

        // extend morph target
        if (has(prim, "targets")) {
          auto &targets = prim.at("targets");
          for (int i = 0; i < targets.size(); ++i) {
            auto &target = targets[i];
            auto morph = ptr->getOrCreateMorphTarget(i);
            // std::cout << target << std::endl;
            auto positions =
                m_gltf.accessor<DirectX::XMFLOAT3>(target.at(VERTEX_POSITION));
            std::vector<DirectX::XMFLOAT3> copy;
            if (m_vrm0) {
              copy.reserve(positions.size());
              for (auto &p : positions) {
                copy.push_back({-p.x, p.y, -p.z});
              }
              positions = copy;
            }
            auto morphOffset = morph->addPosition(positions);
          }
        }

        // extend indices and add vertex offset
        AddIndices(offset, ptr.get(), prim["indices"], material);
      }

      // find morph target name
      // 1. primitive.extras.targetNames
      if (has(prim, "extras")) {
        auto &extras = prim.at("extras");
        if (has(extras, "targetNames")) {
          auto &names = extras.at("targetNames");
          // std::cout << names << std::endl;
          for (int i = 0; i < names.size(); ++i) {
            ptr->getOrCreateMorphTarget(i)->name = names[i];
          }
        }
      }

      lastAtributes = &attributes;
    }
  }

  if (has(m_gltf.json, "skins")) {
    auto skins = m_gltf.json["skins"];
    for (int i = 0; i < skins.size(); ++i) {
      auto &skin = skins[i];
      auto ptr = std::make_shared<gltf::Skin>();
      m_skins.push_back(ptr);

      ptr->name = skin.value("name", std::format("skin{}", i));

      for (auto &joint : skin["joints"]) {
        ptr->joints.push_back(joint);
      }

      auto matrices =
          m_gltf.accessor<DirectX::XMFLOAT4X4>(skin["inverseBindMatrices"]);
      std::vector<DirectX::XMFLOAT4X4> copy;
      if (m_vrm0) {
        copy.reserve(matrices.size());
        for (auto &m : matrices) {
          copy.push_back(m);
          copy.back()._41 = -m._41;
          copy.back()._43 = -m._43;
        }
        matrices = copy;
      }
      ptr->bindMatrices.assign(matrices.begin(), matrices.end());

      assert(ptr->joints.size() == ptr->bindMatrices.size());

      if (has(skin, "skeleton")) {
        ptr->root = skin.at("skeleton");
      }
    }
  }

  auto nodes = m_gltf.json["nodes"];
  for (int i = 0; i < nodes.size(); ++i) {
    auto &node = nodes[i];
    auto name = node.value("name", std::format("node:{}", i));
    auto ptr = std::make_shared<gltf::Node>(i, name);
    m_nodes.push_back(ptr);

    if (has(node, "matrix")) {
      // matrix
      auto m = node["matrix"];
      auto local = DirectX::XMFLOAT4X4{
          m[0],  m[1],  m[2],  m[3],  //
          m[4],  m[5],  m[6],  m[7],  //
          m[8],  m[9],  m[10], m[11], //
          m[12], m[13], m[14], m[15], //
      };
      ptr->setLocalMatrix(local);
    } else {
      // T
      ptr->translation = node.value("translation", DirectX::XMFLOAT3{0, 0, 0});
      if (m_vrm0) {
        auto t = ptr->translation;
        ptr->translation = {-t.x, t.y, -t.z};
      }
      // R
      ptr->rotation = node.value("rotation", DirectX::XMFLOAT4{0, 0, 0, 1});
      // S
      ptr->scale = node.value("scale", DirectX::XMFLOAT3{1, 1, 1});
    }
    if (has(node, "mesh")) {
      auto mesh_index = node.at("mesh");
      ptr->mesh = mesh_index;

      if (has(node, "skin")) {
        int skin_index = node.at("skin");
        auto skin = m_skins[skin_index];
        ptr->skin = skin;
      }

      ptr->Instance =
          std::make_shared<gltf::MeshInstance>(m_meshes[mesh_index]);
    }
  }
  for (int i = 0; i < nodes.size(); ++i) {
    auto &node = nodes[i];
    if (has(node, "children")) {
      for (auto child : node.at("children")) {
        gltf::Node::addChild(m_nodes[i], m_nodes[child]);
      }
    }
  }
  if (has(m_gltf.json, "scenes")) {
    auto scene = m_gltf.json["scenes"][0];
    for (auto &node : scene["nodes"]) {
      m_roots.push_back(m_nodes[node]);
    }
  }

  // calc world
  auto enter = [](gltf::Node &node) {
    node.calcWorld();
    node.init();
    return true;
  };
  Traverse(enter, {});

  auto &animations = m_gltf.json["animations"];
  for (int i = 0; i < animations.size(); ++i) {
    auto &animation = animations[i];
    auto ptr = std::make_shared<gltf::Animation>(
        animation.value("name", std::format("animation:{}", i)));
    m_animations.push_back(ptr);

    // samplers
    auto &samplers = animation["samplers"];

    // channels
    auto &channels = animation["channels"];
    for (auto &channel : channels) {
      int sampler_index = channel.at("sampler");
      auto sampler = samplers[sampler_index];

      auto target = channel.at("target");
      int node_index = target.at("node");
      std::string_view path = target.at("path");

      // time
      int input_index = sampler.at("input");
      auto times = m_gltf.accessor<float>(input_index);
      int output_index = sampler.at("output");

      if (path == "translation") {
        auto values = m_gltf.accessor<DirectX::XMFLOAT3>(output_index);
        ptr->addTranslation(
            node_index, times, values,
            std::format("{}-translation", m_nodes[node_index]->name));
      } else if (path == "rotation") {
        auto values = m_gltf.accessor<DirectX::XMFLOAT4>(output_index);
        ptr->addRotation(node_index, times, values,
                         std::format("{}-rotation", m_nodes[node_index]->name));
      } else if (path == "scale") {
        auto values = m_gltf.accessor<DirectX::XMFLOAT3>(output_index);
        ptr->addScale(node_index, times, values,
                      std::format("{}-scale", m_nodes[node_index]->name));
      } else if (path == "weights") {
        // TODO: not implemented
        auto values = m_gltf.accessor<float>(output_index);
        auto node = m_nodes[node_index];
        if (auto mesh_index = node->mesh) {
          auto mesh = m_meshes[*mesh_index];
          if (values.size() != mesh->m_morphTargets.size() * times.size()) {
            return std::unexpected{"animation-weights: size not match"};
          }
          ptr->addWeights(node_index, times, values,
                          std::format("{}-weights", node->name));
        } else {
          return std::unexpected{"animation-weights: no node.mesh"};
        }
      } else {
        return std::unexpected{
            std::format("animation path {} is not implemented", path)};
      }
    }

    // set animation duration
    // ptr->m_duration = ptr->duration();
  }

  // vrm-0.x
  if (has(m_gltf.json, "extensions")) {
    auto &extensions = m_gltf.json.at("extensions");
    if (has(extensions, "VRM")) {
      auto VRM = extensions.at("VRM");
      // m_vrm0 = std::make_shared<Vrm0>();

      if (has(VRM, "humanoid")) {
        auto &humanoid = VRM.at("humanoid");
        if (has(humanoid, "humanBones")) {
          auto &humanBones = humanoid.at("humanBones");
          // bone & node
          for (auto &humanBone : humanBones) {
            int index = humanBone.at("node");
            std::string_view name = humanBone.at("bone");
            // std::cout << name << ": " << index << std::endl;
            m_vrm0->m_humanoid.setNode(name, vrm::VrmVersion::_0_x, index);
          }
          std::cout << m_vrm0->m_humanoid << std::endl;
        }
      }

      // meta
      // specVersion
      // exporterVersion

      // firstPerson
      // materialProperties

      if (has(VRM, "blendShapeMaster")) {
        auto &blendShapeMaster = VRM.at("blendShapeMaster");
        if (has(blendShapeMaster, "blendShapeGroups")) {
          auto &blendShapeGroups = blendShapeMaster.at("blendShapeGroups");
          for (auto &g : blendShapeGroups) {
            // {"binds":[],"isBinary":false,"materialValues":[],"name":"Neutral","presetName":"neutral"}
            // std::cout << g << std::endl;
            auto expression = m_vrm0->addBlendShape(
                g.at("presetName"), g.at("name"), g.value("isBinary", false));
            if (has(g, "binds")) {
              for (vrm0::ExpressionMorphTargetBind bind : g.at("binds")) {
                // [0-100] to [0-1]
                bind.weight *= 0.01f;
                expression->morphBinds.push_back(bind);
              }
            }
          }
        }
      }

      if (has(VRM, "secondaryAnimation")) {
        auto &secondaryAnimation = VRM.at("secondaryAnimation");
        if (has(secondaryAnimation, "colliderGroups")) {
          auto &colliderGroups = secondaryAnimation.at("colliderGroups");
          for (auto &colliderGroup : colliderGroups) {
            auto ptr = std::make_shared<vrm0::ColliderGroup>();
            *ptr = colliderGroup;
            std::cout << *ptr << std::endl;
            m_vrm0->m_colliderGroups.push_back(ptr);
          }
        }
        if (has(secondaryAnimation, "boneGroups")) {
          auto &boneGroups = secondaryAnimation.at("boneGroups");
          for (auto &boneGroup : boneGroups) {
            auto ptr = std::make_shared<vrm0::Spring>();
            *ptr = boneGroup;
            std::cout << *ptr << std::endl;
            m_vrm0->m_springs.push_back(ptr);
          }
        }

        m_spring->Clear();
        for (auto &spring : m_vrm0->m_springs) {
          for (auto node_index : spring->bones) {
            m_spring->Add(m_nodes[node_index], spring->dragForce,
                          spring->stiffiness);
          }
        }
      }
    }
  }

  return {};
}

void Scene::AddIndices(int vertex_offset, gltf::Mesh *mesh, int accessor_index,
                       const std::shared_ptr<gltf::Material> &material) {
  auto accessor = m_gltf.json["accessors"][accessor_index];
  switch ((ComponentType)accessor["componentType"]) {
  case ComponentType::UNSIGNED_BYTE: {
    auto span = m_gltf.accessor<uint8_t>(accessor_index);
    mesh->addSubmesh(vertex_offset, span, material);
  } break;
  case ComponentType::UNSIGNED_SHORT: {
    auto span = m_gltf.accessor<uint16_t>(accessor_index);
    mesh->addSubmesh(vertex_offset, span, material);
  } break;
  case ComponentType::UNSIGNED_INT: {
    auto span = m_gltf.accessor<uint32_t>(accessor_index);
    mesh->addSubmesh(vertex_offset, span, material);
  } break;
  default:
    throw std::runtime_error("invalid index type");
  }
}

void Scene::Render(Time time, const Camera &camera, const RenderFunc &render) {
  SyncHierarchy();

  // springbone
  if (m_vrm0) {
    m_spring->Update(time);
  }

  if (m_vrm0) {
    // VRM0 expression to morphTarget
    auto meshToNode = [nodes = m_nodes](uint32_t mi) {
      for (auto &node : nodes) {
        if (node->mesh == mi) {
          return (uint32_t)node->index;
        }
      }
      return (uint32_t)-1;
    };
    for (auto &[k, v] : m_vrm0->EvalMorphTargetMap(meshToNode)) {
      auto &morph_node = m_nodes[k.NodeIndex];
      morph_node->Instance->weights[k.MorphIndex] = v;
    }
  }

  // skinning

  for (auto &node : m_nodes) {
    if (auto mesh_index = node->mesh) {
      auto mesh = m_meshes[*mesh_index];

      // mesh animation
      std::span<DirectX::XMFLOAT4X4> skinningMatrices;

      // skinning
      if (auto skin = node->skin) {
        skin->currentMatrices.resize(skin->bindMatrices.size());

        auto rootInverse = DirectX::XMMatrixIdentity();
        if (auto root_index = skin->root) {
          rootInverse = DirectX::XMMatrixInverse(
              nullptr, DirectX::XMLoadFloat4x4(&node->world));
        }

        for (int i = 0; i < skin->joints.size(); ++i) {
          auto node = m_nodes[skin->joints[i]];
          auto m = skin->bindMatrices[i];
          DirectX::XMStoreFloat4x4(&skin->currentMatrices[i],
                                   DirectX::XMLoadFloat4x4(&m) *
                                       DirectX::XMLoadFloat4x4(&node->world) *
                                       rootInverse);
        }

        skinningMatrices = skin->currentMatrices;
      }

      // apply morphtarget & skinning
      node->Instance->applyMorphTargetAndSkinning(*mesh, skinningMatrices);
    }
  }

  for (auto &node : m_nodes) {
    if (auto mesh_index = node->mesh) {
      auto mesh = m_meshes[*mesh_index];
      render(camera, *mesh, *node->Instance, &node->world._11);
    }
  }

  if (m_vrm0) {
    m_spring->DrawGizmo();
  }
}

void Scene::Traverse(const EnterFunc &enter, const LeaveFunc &leave,
                     gltf::Node *node) {
  if (node) {
    if (enter(*node)) {
      for (auto &child : node->children) {
        Traverse(enter, leave, child.get());
      }
      if (leave) {
        leave();
      }
    }
  } else {
    // root
    for (auto &child : m_roots) {
      Traverse(enter, leave, child.get());
    }
  }
}

void Scene::TraverseJson(const EnterJson &enter, const LeaveJson &leave,
                         nlohmann::json *item, std::string_view key) {
  if (!item) {
    // root
    for (auto &kv : m_gltf.json.items()) {
      TraverseJson(enter, leave, &kv.value(), kv.key());
    }
    return;
  }

  if (enter(*item, key.size() ? std::string{key.begin(), key.end()} : "")) {
    if (item->is_object()) {
      for (auto &kv : item->items()) {
        TraverseJson(enter, leave, &kv.value(), kv.key());
      }
    } else if (item->is_array()) {
      for (int i = 0; i < item->size(); ++i) {
        TraverseJson(enter, leave, &(*item)[i], std::format("{}", i));
      }
    }
    if (leave) {
      leave();
    }
  }
}

void Scene::SetHumanPose(std::span<const vrm::HumanBones> humanMap,
                         const DirectX::XMFLOAT3 &rootPosition,
                         std::span<const DirectX::XMFLOAT4> rotations) {

  assert(humanMap.size() == rotations.size());

  if (m_vrm0) {
    for (int i = 0; i < humanMap.size(); ++i) {
      if (auto node = GetBoneNode(humanMap[i])) {
        if (i == 0) {
          node->translation = rootPosition;
        }
        node->rotation = rotations[i];
      }
    }
  }
}

void Scene::SyncHierarchy() {
  // calc world
  auto enter = [](gltf::Node &node) {
    node.calcWorld();
    return true;
  };
  Traverse(enter, {});
}

std::shared_ptr<gltf::Node> Scene::GetBoneNode(vrm::HumanBones bone) {
  if (auto node_index = m_vrm0->m_humanoid[(int)bone]) {
    return m_nodes[*node_index];
  }
  return {};
}

std::vector<uint8_t> Scene::ToGlb() const {
  std::vector<uint8_t> bytes;
  return bytes;
}
