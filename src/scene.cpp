#include "scene.h"
#include "material.h"
#include "mesh.h"
#include <DirectXMath.h>
#include <array>
#include <expected>
#include <format>
#include <fstream>
#include <iostream>
#include <list>
#include <nlohmann/json.hpp>
#include <optional>
#include <span>
#include <string>
#include <vector>

using json = nlohmann::json;

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

class BinaryReader {
  std::span<const uint8_t> m_data;
  size_t m_pos = 0;

public:
  BinaryReader(std::span<const uint8_t> data) : m_data(data) {}

  template <typename T> T get() {
    auto value = *((T *)&m_data[m_pos]);
    m_pos += sizeof(T);
    return value;
  }

  void resize(size_t len) { m_data = std::span(m_data.begin(), len); }

  std::span<const uint8_t> span(size_t size) {
    auto value = m_data.subspan(m_pos, size);
    m_pos += size;
    return value;
  }

  std::string_view string_view(size_t size) {
    auto value = m_data.subspan(m_pos, size);
    m_pos += size;
    return std::string_view((const char *)value.data(),
                            (const char *)value.data() + value.size());
  }

  bool is_end() const { return m_pos >= m_data.size(); }
};

static std::expected<size_t, std::string> component_size(int component_type) {
  switch (component_type) {
  case 5120: // BYTE
    return 1;
  case 5121: // UNSIGNED_BYTE
    return 1;
  case 5122: // SHORT
    return 2;
  case 5123: // UNSIGNED_SHORT
    return 2;
  case 5125: // UNSIGNED_INT
    return 4;
  case 5126: // FLOAT
    return 4;
  default:
    return std::unexpected{"invalid component type"};
  }
}

static std::expected<size_t, std::string> type_size(std::string_view type) {
  if (type == "SCALAR") {
    return 1;
  } else if (type == "VEC2") {
    return 2;
  } else if (type == "VEC3") {
    return 3;
  } else if (type == "VEC4") {
    return 4;
  } else if (type == "MAT2") {
    return 4;
  } else if (type == "MAT3") {
    return 9;
  } else if (type == "MAT4") {
    return 16;
  } else {
    return std::unexpected{std::string(type)};
  }
}

static std::expected<size_t, std::string> item_size(const json &accessor) {
  if (auto cs = component_size(accessor["componentType"])) {
    if (auto ts = type_size(accessor["type"])) {
      return *cs * *ts;
    } else {
      return ts;
    }
  } else {
    throw cs;
  }
}

struct Glb {
  json gltf;
  std::span<const uint8_t> bin;

  std::span<const uint8_t> buffer_view(int buffer_view_index) {
    auto buffer_view = gltf["bufferViews"][buffer_view_index];
    // std::cout << buffer_view << std::endl;
    return bin.subspan(buffer_view["byteOffset"], buffer_view["byteLength"]);
  }

  template <typename T> std::span<const T> accessor(int accessor_index) {
    auto accessor = gltf["accessors"][accessor_index];
    // std::cout << accessor << std::endl;
    assert(*item_size(accessor) == sizeof(T));
    auto span = buffer_view(accessor["bufferView"]);

    int offset = accessor.value("byteOffset", 0);
    return std::span<const T>((const T *)(span.data() + offset),
                              accessor["count"]);
  }

  std::tuple<std::span<const uint8_t>, uint32_t> indices(int accessor_index) {
    auto accessor = gltf["accessors"][accessor_index];
    // std::cout << accessor << std::endl;
    auto span = buffer_view(accessor["bufferView"]);
    return {span, accessor["count"]};
  }
};
std::optional<Glb> parseGlb(std::span<const uint8_t> bytes) {

  BinaryReader r(bytes);
  if (r.get<uint32_t>() != 0x46546C67) {
    return {};
  }

  if (r.get<uint32_t>() != 2) {
    return {};
  }

  auto length = r.get<uint32_t>();
  r.resize(length);

  Glb glb{};
  {
    auto chunk_length = r.get<uint32_t>();
    if (r.get<uint32_t>() != 0x4E4F534A) {
      // first chunk must "JSON"
      return {};
    }
    auto gltf = r.string_view(chunk_length);
    glb.gltf = json::parse(gltf);
  }
  if (!r.is_end()) {
    auto chunk_length = r.get<uint32_t>();
    if (r.get<uint32_t>() != 0x004E4942) {
      // second chunk is "BIN"
      return {};
    }
    glb.bin = r.span(chunk_length);
  }

  return glb;
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

  auto glb = parseGlb(bytes);
  if (!glb) {
    return;
  }

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
}

void Scene::render(const Camera &camera, const RenderFunc &render) {
  // update
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
