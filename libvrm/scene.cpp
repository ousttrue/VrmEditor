#include "vrm/scene.h"
#include "vrm/animation.h"
#include "vrm/glb.h"
#include "vrm/material.h"
#include "vrm/mesh.h"
#include "vrm/node.h"
#include "vrm/skin.h"
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

const auto VERTEX_JOINT = "JOINTS_0";
const auto VERTEX_WEIGHT = "WEIGHTS_0";
const auto VERTEX_POSITION = "POSITION";
const auto VERTEX_NORMAL = "NORMAL";
const auto VERTEX_UV = "TEXCOORD_0";

static DirectX::XMFLOAT4X4 IDENTITY{
    1, 0, 0, 0, //
    0, 1, 0, 0, //
    0, 0, 1, 0, //
    0, 0, 0, 1, //
};

// float3
inline void to_json(json &j, const float3 &v) { j = json{v.x, v.y, v.z}; }
inline void from_json(const json &j, float3 &v) {
  v.x = j[0];
  v.y = j[1];
  v.z = j[2];
}

// quaternion
inline void to_json(json &j, const quaternion &v) {
  j = json{v.x, v.y, v.z, v.w};
}
inline void from_json(const json &j, quaternion &v) {
  v.x = j[0];
  v.y = j[1];
  v.z = j[2];
  v.w = j[3];
}

// matrix
inline void to_json(json &j, const DirectX::XMFLOAT4X4 &m) {
  j = json{
      m._11, m._12, m._13, m._14, m._21, m._22, m._23, m._24,
      m._31, m._32, m._33, m._34, m._41, m._42, m._43, m._44,
  };
}
inline void from_json(const json &j, DirectX::XMFLOAT4X4 &m) {
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

inline void from_json(const json &j, Vrm0ExpressionMorphTargetBind &b) {
  b.mesh = j.at("mesh");
  b.index = j.at("index");
  b.weight = j.at("weight");
}

template <typename T>
static std::vector<T> ReadAllBytes(const std::filesystem::path &path) {
  std::ifstream ifs(path, std::ios::binary | std::ios::ate);
  if (!ifs) {
    return {};
  }
  auto pos = ifs.tellg();
  auto size = pos / sizeof(T);
  if (pos % sizeof(T)) {
    ++size;
  }
  std::vector<T> buffer(size);
  ifs.seekg(0, std::ios::beg);
  ifs.read((char *)buffer.data(), pos);
  return buffer;
}

bool Scene::load(const std::filesystem::path &path) {
  auto bytes = ReadAllBytes<uint8_t>(path);
  if (bytes.empty()) {
    return false;
  }

  auto glb = Glb::parse(bytes);
  if (!glb) {
    return false;
  }

  m_gltf = glb->gltf;

  auto &images = glb->gltf["images"];
  for (int i = 0; i < images.size(); ++i) {
    auto &image = images[i];
    auto bytes = glb->buffer_view(image["bufferView"]);

    auto ptr =
        std::make_shared<Image>(image.value("name", std::format("image{}", i)));
    if (ptr->load(bytes)) {

    } else {
    }
    m_images.push_back(ptr);
  }

  auto &textures = glb->gltf["textures"];

  auto &materials = glb->gltf["materials"];
  for (int i = 0; i < materials.size(); ++i) {
    auto &material = materials[i];
    auto ptr = std::make_shared<Material>(
        material.value("name", std::format("material{}", i)));
    m_materials.push_back(ptr);
    if (material.find("pbrMetallicRoughness") != material.end()) {
      auto pbrMetallicRoughness = material.at("pbrMetallicRoughness");
      if (pbrMetallicRoughness.find("baseColorTexture") !=
          pbrMetallicRoughness.end()) {
        auto &baseColorTexture = pbrMetallicRoughness.at("baseColorTexture");
        int texture_index = baseColorTexture.at("index");
        auto texture = textures.at(texture_index);
        int image_index = texture["source"];
        ptr->texture = m_images[image_index];
      }
    }
  }

  for (auto &mesh : glb->gltf["meshes"]) {
    auto ptr = std::make_shared<Mesh>();
    m_meshes.push_back(ptr);

    json *lastAtributes = nullptr;
    for (auto &prim : mesh["primitives"]) {
      json &attributes = prim.at("attributes");

      if (lastAtributes && attributes == *lastAtributes) {
        // for vrm shared vertex buffer
        addIndices(0, ptr.get(), &*glb, prim.at("indices"),
                   prim.at("material"));
      } else {
        // extend vertex buffer
        auto offset = ptr->addPosition(
            glb->accessor<float3>(attributes[VERTEX_POSITION]));
        if (attributes.find(VERTEX_NORMAL) != attributes.end()) {
          ptr->setNormal(offset,
                         glb->accessor<float3>(attributes.at(VERTEX_NORMAL)));
        }
        if (attributes.find(VERTEX_UV) != attributes.end()) {
          ptr->setUv(offset, glb->accessor<float2>(attributes.at(VERTEX_UV)));
        }

        if (attributes.find(VERTEX_JOINT) != attributes.end() &&
            attributes.find(VERTEX_WEIGHT) != attributes.end()) {
          // skinning
          ptr->setBoneSkinning(
              offset, glb->accessor<ushort4>(attributes.at(VERTEX_JOINT)),
              glb->accessor<float4>(attributes.at(VERTEX_WEIGHT)));
        }

        // extend morph target
        if (prim.find("targets") != prim.end()) {
          auto &targets = prim.at("targets");
          for (int i = 0; i < targets.size(); ++i) {
            auto &target = targets[i];
            auto morph = ptr->getOrCreateMorphTarget(i);
            // std::cout << target << std::endl;
            auto morphOffset = morph->addPosition(
                glb->accessor<float3>(target.at(VERTEX_POSITION)));
          }
        }

        // extend indices and add vertex offset
        addIndices(offset, ptr.get(), &*glb, prim["indices"],
                   prim.at("material"));
      }

      // find morph target name
      // 1. primitive.extras.targetNames
      if (prim.find("extras") != prim.end()) {
        auto &extras = prim.at("extras");
        if (extras.find("targetNames") != extras.end()) {
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

  if (glb->gltf.find("skins") != glb->gltf.end()) {
    auto skins = glb->gltf["skins"];
    for (int i = 0; i < skins.size(); ++i) {
      auto &skin = skins[i];
      auto ptr = std::make_shared<Skin>();
      m_skins.push_back(ptr);

      ptr->name = skin.value("name", std::format("skin{}", i));

      for (auto &joint : skin["joints"]) {
        ptr->joints.push_back(joint);
      }

      auto matrices =
          glb->accessor<DirectX::XMFLOAT4X4>(skin["inverseBindMatrices"]);
      ptr->bindMatrices.assign(matrices.begin(), matrices.end());

      assert(ptr->joints.size() == ptr->bindMatrices.size());

      if (skin.find("skeleton") != skin.end()) {
        ptr->root = skin.at("skeleton");
      }
    }
  }

  auto nodes = glb->gltf["nodes"];
  for (int i = 0; i < nodes.size(); ++i) {
    auto &node = nodes[i];
    auto name = node.value("name", std::format("node:{}", i));
    auto ptr = std::make_shared<Node>(i, name);
    m_nodes.push_back(ptr);

    if (node.find("matrix") != node.end()) {
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
      ptr->translation = node.value("translation", float3{0, 0, 0});
      // R
      ptr->rotation = node.value("rotation", quaternion{0, 0, 0, 1});
      // S
      ptr->scale = node.value("scale", float3{1, 1, 1});
    }
    if (node.find("mesh") != node.end()) {
      ptr->mesh = node.at("mesh");

      if (node.find("skin") != node.end()) {
        int skin_index = node.at("skin");
        auto skin = m_skins[skin_index];
        ptr->skin = skin;
      }
    }
  }
  for (int i = 0; i < nodes.size(); ++i) {
    auto &node = nodes[i];
    if (node.find("children") != node.end()) {
      for (auto child : node.at("children")) {
        m_nodes[i]->addChild(m_nodes[child]);
      }
    }
  }

  if (glb->gltf.find("scenes") != glb->gltf.end()) {
    auto scene = glb->gltf["scenes"][0];
    for (auto &node : scene["nodes"]) {
      m_roots.push_back(m_nodes[node]);
    }
  }

  auto &animations = glb->gltf["animations"];
  for (int i = 0; i < animations.size(); ++i) {
    auto &animation = animations[i];
    auto ptr = std::make_shared<Animation>(
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
      auto times = glb->accessor<float>(input_index);
      int output_index = sampler.at("output");

      if (path == "translation") {
        auto values = glb->accessor<float3>(output_index);
        ptr->addTranslation(
            node_index, times, values,
            std::format("{}-translation", m_nodes[node_index]->name));
      } else if (path == "rotation") {
        auto values = glb->accessor<quaternion>(output_index);
        ptr->addRotation(node_index, times, values,
                         std::format("{}-rotation", m_nodes[node_index]->name));
      } else if (path == "scale") {
        auto values = glb->accessor<float3>(output_index);
        ptr->addScale(node_index, times, values,
                      std::format("{}-scale", m_nodes[node_index]->name));
      } else {
        assert(false);
      }
    }

    // set animation duration
    ptr->m_duration = ptr->duration();
  }

  // vrm-0.x
  if (glb->gltf.find("extensions") != glb->gltf.end()) {
    auto &extensions = glb->gltf.at("extensions");
    if (extensions.find("VRM") != extensions.end()) {
      auto VRM = extensions.at("VRM");
      m_vrm0 = std::make_shared<Vrm0>();

      if (VRM.find("humanoid") != VRM.end()) {
        auto &humanoid = VRM.at("humanoid");
        if (humanoid.find("humanBones") != humanoid.end()) {
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

      // exporterVersion
      // firstPerson
      // materialProperties
      // meta
      // secondaryAnimation
      // specVersion

      if (VRM.find("blendShapeMaster") != VRM.end()) {
        auto &blendShapeMaster = VRM.at("blendShapeMaster");
        if (blendShapeMaster.find("blendShapeGroups") !=
            blendShapeMaster.end()) {
          auto &blendShapeGroups = blendShapeMaster.at("blendShapeGroups");
          for (auto &g : blendShapeGroups) {
            // {"binds":[],"isBinary":false,"materialValues":[],"name":"Neutral","presetName":"neutral"}
            // std::cout << g << std::endl;
            auto expression = m_vrm0->addBlendShape(
                g.at("presetName"), g.at("name"), g.value("isBinary", false));
            if (g.find("binds") != g.end()) {
              for (Vrm0ExpressionMorphTargetBind bind : g.at("binds")) {
                // [0-100] to [0-1]
                bind.weight *= 0.01f;
                expression->morphBinds.push_back(bind);
              }
            }
          }
        }
      }
    }
  }

  // calc world
  auto enter = [](Node &node, const DirectX::XMFLOAT4X4 &parent) {
    node.calcWorld(parent);
    node.worldInit = node.world;
    return true;
  };
  traverse(enter, {});

  return true;
}

void Scene::addIndices(int vertex_offset, Mesh *mesh, Glb *glb,
                       int accessor_index, int material_index) {
  auto accessor = glb->gltf["accessors"][accessor_index];
  switch ((ComponentType)accessor["componentType"]) {
  case ComponentType::UNSIGNED_BYTE: {
    auto span = glb->accessor<uint8_t>(accessor_index);
    mesh->addSubmesh(vertex_offset, span, m_materials[material_index]);
  } break;
  case ComponentType::UNSIGNED_SHORT: {
    auto span = glb->accessor<uint16_t>(accessor_index);
    mesh->addSubmesh(vertex_offset, span, m_materials[material_index]);
  } break;
  case ComponentType::UNSIGNED_INT: {
    auto span = glb->accessor<uint32_t>(accessor_index);
    mesh->addSubmesh(vertex_offset, span, m_materials[material_index]);
  } break;
  default:
    throw std::runtime_error("invalid index type");
  }
}

void Scene::update() {
  if (m_updated) {
    return;
  }

  // calc world
  auto enter = [](Node &node, const DirectX::XMFLOAT4X4 &parent) {
    node.calcWorld(parent);
    return true;
  };
  traverse(enter, {});

  // skinning
  for (auto &node : m_nodes) {
    if (auto mesh_index = node->mesh) {
      auto mesh = m_meshes[*mesh_index];

      // skinning
      if (auto skin = node->skin) {
        skin->currentMatrices.resize(skin->bindMatrices.size());

        auto root = DirectX::XMMatrixIdentity();
        auto rootInverse = root;
        auto rootTranslation = root;
        if (auto root_index = skin->root) {
          // auto rootNode = m_nodes[*root_index];
          // // rotation only ???
          // auto world = rootNode->worldInit;
          // world._41 = 0;
          // world._42 = 0;
          // world._43 = 0;
          // root = DirectX::XMLoadFloat4x4(&world);
          // rootInverse = DirectX::XMMatrixInverse(nullptr, root);
          // rootTranslation = DirectX::XMMatrixTranslation(
          //     rootNode->world._41, rootNode->world._42, rootNode->world._43);
          root = DirectX::XMLoadFloat4x4(&node->world);
          rootInverse = DirectX::XMMatrixInverse(nullptr, root);
        }

        for (int i = 0; i < skin->joints.size(); ++i) {
          auto node = m_nodes[skin->joints[i]];
          auto m = skin->bindMatrices[i];
          DirectX::XMStoreFloat4x4(&skin->currentMatrices[i],
                                   DirectX::XMLoadFloat4x4(&m) *
                                       DirectX::XMLoadFloat4x4(&node->world) *
                                       rootInverse);
        }

        if (m_vrm0) {
          // VRM0 expression to morphTarget

          // clear
          for (auto &mesh : m_meshes) {
            for (auto &morph : mesh->m_morphTargets) {
              morph->weight = 0;
            }
          }
          // apply
          for (auto &expression : m_vrm0->m_expressions) {
            for (auto &morphTarget : expression->morphBinds) {
              auto &mesh = m_meshes[morphTarget.mesh];
              auto &morph = mesh->m_morphTargets[morphTarget.index];
              morph->weight += morphTarget.weight * expression->weight;
            }
          }
        }

        mesh->applyMorphTargetAndSkinning(skin->currentMatrices);
      }
    }
  }
  m_updated = true;
}

void Scene::render(const Camera &camera, const RenderFunc &render) {
  update();

  // render mesh
  for (auto &node : m_nodes) {
    if (auto mesh_index = node->mesh) {
      auto mesh = m_meshes[*mesh_index];
      render(camera, *mesh, &node->world._11);
    }
  }

  m_updated = false;
}

void Scene::traverse(const EnterFunc &enter, const LeaveFunc &leave, Node *node,
                     const DirectX::XMFLOAT4X4 &parent) {
  if (node) {
    if (enter(*node, parent)) {
      for (auto &child : node->children) {
        traverse(enter, leave, child.get(), node->world);
      }
      if (leave) {
        leave();
      }
    }
  } else {
    // root
    for (auto &child : m_roots) {
      traverse(enter, leave, child.get(), IDENTITY);
    }
  }
}

void Scene::traverse_json(const EnterJson &enter, const LeaveJson &leave,
                          json *item, std::string_view key) {
  if (!item) {
    // root
    for (auto &kv : m_gltf.items()) {
      traverse_json(enter, leave, &kv.value(), kv.key());
    }
    return;
  }

  if (enter(*item, key.size() ? std::string{key.begin(), key.end()} : "")) {
    if (item->is_object()) {
      for (auto &kv : item->items()) {
        traverse_json(enter, leave, &kv.value(), kv.key());
      }
    } else if (item->is_array()) {
      for (int i = 0; i < item->size(); ++i) {
        traverse_json(enter, leave, &(*item)[i], std::format("{}", i));
      }
    }
    if (leave) {
      leave();
    }
  }
}

void Scene::SetHumanPose(std::span<const vrm::HumanBones> humanMap,
                         std::span<const DirectX::XMFLOAT4> rotations) {

  assert(humanMap.size() == rotations.size());

  if (m_vrm0) {
    for (int i = 0; i < humanMap.size(); ++i) {
      if (auto node = GetBoneNode(humanMap[i])) {
        node->rotation = *((quaternion *)&rotations[i]);
      }
    }
  }
}

std::shared_ptr<Node> Scene::GetBoneNode(vrm::HumanBones bone) {
  if (auto node_index = m_vrm0->m_humanoid[(int)bone]) {
    return m_nodes[*node_index];
  }
  return {};
}
