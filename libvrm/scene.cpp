#include "vrm/scene.h"
#include "vrm/animation.h"
#include "vrm/glb.h"
#include "vrm/material.h"
#include "vrm/mesh.h"
#include "vrm/node.h"
#include "vrm/skin.h"
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

template <typename T>
static std::vector<T> ReadAllBytes(const std::string &filename) {
  std::ifstream ifs(filename.c_str(), std::ios::binary | std::ios::ate);
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

Node::Node(uint32_t i, std::string_view name) : index(i), name(name) {}

void Node::addChild(const std::shared_ptr<Node> &child) {
  if (auto current_parent = child->parent.lock()) {
    current_parent->children.remove(child);
  }
  child->parent = shared_from_this();
  children.push_back(child);
}

void Node::calcWorld(const DirectX::XMFLOAT4X4 &parent) {
  auto t =
      DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);
  auto r = DirectX::XMMatrixRotationQuaternion(
      DirectX::XMLoadFloat4((DirectX::XMFLOAT4 *)&rotation));
  auto s = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
  DirectX::XMStoreFloat4x4(&world,
                           s * r * t * DirectX::XMLoadFloat4x4(&parent));
}

bool Node::setLocalMatrix(const DirectX::XMFLOAT4X4 &local) {
  DirectX::XMVECTOR s;
  DirectX::XMVECTOR r;
  DirectX::XMVECTOR t;
  if (!DirectX::XMMatrixDecompose(&s, &r, &t,
                                  DirectX::XMLoadFloat4x4(&local))) {
    return false;
  }
  DirectX::XMStoreFloat3((DirectX::XMFLOAT3 *)&scale, s);
  DirectX::XMStoreFloat4((DirectX::XMFLOAT4 *)&rotation, r);
  DirectX::XMStoreFloat3((DirectX::XMFLOAT3 *)&translation, t);
  return true;
}

bool Node::setWorldMatrix(const DirectX::XMFLOAT4X4 &world,
                          const DirectX::XMFLOAT4X4 &parent) {
  auto inv = DirectX::XMMatrixInverse(
      nullptr, DirectX::XMLoadFloat4x4((DirectX::XMFLOAT4X4 *)&parent));
  auto local = DirectX::XMLoadFloat4x4(&world) * inv;

  DirectX::XMFLOAT4X4 m;
  DirectX::XMStoreFloat4x4(&m, local);
  return setLocalMatrix(m);
}

void Node::print(int level) {
  for (int i = 0; i < level; ++i) {
    std::cout << "  ";
  }
  std::cout << name << std::endl;
  for (auto child : children) {
    child->print(level + 1);
  }
}

void Scene::load(const char *path) {
  auto bytes = ReadAllBytes<uint8_t>(path);
  if (bytes.empty()) {
    return;
  }

  auto glb = Glb::parse(bytes);
  if (!glb) {
    return;
  }
  gltf = glb->gltf;

  auto &images = glb->gltf["images"];
  for (int i = 0; i < images.size(); ++i) {
    auto &image = images[i];
    std::cout << image << std::endl;
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
    std::cout << material << std::endl;
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
    for (auto prim : mesh["primitives"]) {
      auto ptr = std::make_shared<Mesh>();
      m_meshes.push_back(ptr);

      json attributes = prim["attributes"];
      auto offset =
          ptr->addPosition(glb->accessor<float3>(attributes[VERTEX_POSITION]));
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

      {
        // std::cout << "indices: " << prim["indices"] << std::endl;
        auto [span, draw_count] = glb->indices(prim["indices"]);

        int material_index = prim.at("material");
        ptr->addSubmesh(offset, span, draw_count, m_materials[material_index]);
      }
    }
  }

  if (glb->gltf.find("skins") != glb->gltf.end()) {
    auto skins = glb->gltf["skins"];
    for (int i = 0; i < skins.size(); ++i) {
      auto &skin = skins[i];
      std::cout << skin << std::endl;
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
      // std::cout << m << std::endl;
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
        ptr->skin = m_skins[skin_index];
      }
    }

    std::cout << *ptr << std::endl;
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
      m_roots.back()->print();
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
      std::cout << node_index << std::endl;
      std::cout << path << std::endl;

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
  }
}

void Scene::render(const Camera &camera, const RenderFunc &render,
                   std::chrono::milliseconds time) {
  // update local
  for (auto &animation : m_animations) {
    animation->update(time, m_nodes);
  }

  // calc world
  auto enter = [](Node &node, const DirectX::XMFLOAT4X4 &parent) {
    node.calcWorld(parent);
    return true;
  };
  traverse(enter, {});

  // render mesh
  for (auto &node : m_nodes) {
    if (auto mesh_index = node->mesh) {
      auto mesh = m_meshes[*mesh_index];

      if (auto skin = node->skin) {
        skin->currentMatrices.resize(skin->bindMatrices.size());

        auto root = DirectX::XMMatrixIdentity();
        auto rootInverse = root;
        auto rootTranslation = root;
        if (auto root_index = skin->root) {
          auto rootNode = m_nodes[*root_index];
          // rotation only ???
          auto world = rootNode->world;
          world._41 = 0;
          world._42 = 0;
          world._43 = 0;
          root = DirectX::XMLoadFloat4x4(&world);
          rootInverse = DirectX::XMMatrixInverse(nullptr, root);
          rootTranslation = DirectX::XMMatrixTranslation(
              rootNode->world._41, rootNode->world._42, rootNode->world._43);
        }

        for (int i = 0; i < skin->joints.size(); ++i) {
          auto node = m_nodes[skin->joints[i]];
          auto m = skin->bindMatrices[i];
          DirectX::XMStoreFloat4x4(&skin->currentMatrices[i],
                                   DirectX::XMLoadFloat4x4(&m) *
                                       DirectX::XMLoadFloat4x4(&node->world) *
                                       rootInverse);
        }
        mesh->skinning(skin->currentMatrices);
      }

      render(camera, *mesh, &node->world._11);
    }
  }
}

void Scene::traverse(const EnterFunc &enter, const LeaveFunc &leave, Node *node,
                     const DirectX::XMFLOAT4X4 &parent) {
  if (node) {
    if (enter(*node, parent)) {
      for (auto &child : node->children) {
        traverse(enter, leave, child.get(), node->world);
      }
      if (leave) {
        leave(*node);
      }
    }
  } else {
    // root
    for (auto &child : m_roots) {
      traverse(enter, leave, child.get(), IDENTITY);
    }
  }
}

void Scene::traverse_json(json *item) {
  if (!item) {
    item = &gltf;
  }
}
