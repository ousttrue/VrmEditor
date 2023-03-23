#pragma once
#include "scenetypes.h"
#include <DirectXMath.h>
#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <ostream>
#include <span>
#include <stdint.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

struct Camera;
struct Mesh;

struct Skin {
  std::string name;
  std::vector<uint32_t> joints;
  std::vector<DirectX::XMFLOAT4X4> bindMatrices;
  // bindMatrix * node worldmatrix
  std::vector<DirectX::XMFLOAT4X4> currentMatrices;
  std::optional<uint32_t> root;
};

struct Node : public std::enable_shared_from_this<Node> {
  uint32_t index;
  std::string name;
  float3 translation = {};
  quaternion rotation = {};
  float3 scale = {};

  DirectX::XMFLOAT4X4 world;

  std::optional<uint32_t> mesh;
  std::shared_ptr<Skin> skin;

  std::list<std::shared_ptr<Node>> children;
  std::weak_ptr<Node> parent;

  Node(uint32_t i, std::string_view name);
  Node(const Node &) = delete;
  Node &operator=(const Node &) = delete;
  void addChild(const std::shared_ptr<Node> &child);
  void calcWorld(const DirectX::XMFLOAT4X4 &parent);
  bool setLocalMatrix(const DirectX::XMFLOAT4X4 &local);
  bool setWorldMatrix(const DirectX::XMFLOAT4X4 &world,
                      const DirectX::XMFLOAT4X4 &parent);
  void print(int level = 0);
};
inline std::ostream &operator<<(std::ostream &os, const Node &node) {
  os << "Node[" << node.index << "]" << node.name << ": " << node.translation
     << node.rotation << node.scale;
  if (node.mesh) {
    os << ", mesh: " << *node.mesh;
  }
  return os;
}

template <typename T> struct Curve {
  std::vector<float> times;
  std::vector<T> values;

  T getValue(float time) const {
    while (time > times.back()) {
      time -= times.back();
      if (time < 0) {
        time = 0;
      }
    }
    for (int i = 0; i < times.size(); ++i) {
      if (times[i] > time) {
        return values[i];
      }
    }
    return values.back();
  }
};

class Animation {
  std::unordered_map<uint32_t, Curve<float3>> m_translationMap;
  std::unordered_map<uint32_t, Curve<quaternion>> m_rotationMap;
  std::unordered_map<uint32_t, Curve<float3>> m_scaleMap;

public:
  void addTranslation(uint32_t node_index, std::span<const float> times,
                      std::span<const float3> values) {
    m_translationMap.emplace(node_index,
                             Curve<float3>{
                                 .times = {times.begin(), times.end()},
                                 .values = {values.begin(), values.end()},
                             });
  }
  void addRotation(uint32_t node_index, std::span<const float> times,
                   std::span<const quaternion> values) {
    m_rotationMap.emplace(node_index,
                          Curve<quaternion>{
                              .times = {times.begin(), times.end()},
                              .values = {values.begin(), values.end()},
                          });
  }
  void addScale(uint32_t node_index, std::span<const float> times,
                std::span<const float3> values) {
    m_scaleMap.emplace(node_index, Curve<float3>{
                                       .times = {times.begin(), times.end()},
                                       .values = {values.begin(), values.end()},
                                   });
  }

  void update(std::chrono::milliseconds time,
              std::span<std::shared_ptr<Node>> nodes) {
    float seconds = time.count() * 0.001f;
    for (auto &[k, v] : m_translationMap) {
      auto node = nodes[k];
      node->translation = v.getValue(seconds);
    }
    for (auto &[k, v] : m_rotationMap) {
      auto node = nodes[k];
      node->rotation = v.getValue(seconds);
    }
    for (auto &[k, v] : m_scaleMap) {
      auto node = nodes[k];
      node->scale = v.getValue(seconds);
    }
  }
};

class Image;
class Material;
using RenderFunc =
    std::function<void(const Camera &, const Mesh &, const float[16])>;

using EnterFunc = std::function<bool(Node &, const DirectX::XMFLOAT4X4 &)>;
using LeaveFunc = std::function<void(Node &)>;

class Scene {
  std::vector<std::shared_ptr<Image>> m_images;
  std::vector<std::shared_ptr<Material>> m_materials;
  std::vector<std::shared_ptr<Mesh>> m_meshes;
  std::vector<std::shared_ptr<Node>> m_nodes;
  std::vector<std::shared_ptr<Node>> m_roots;
  std::vector<std::shared_ptr<Skin>> m_skins;
  std::vector<std::shared_ptr<Animation>> m_animations;

public:
  Scene() {}
  Scene(const Scene &) = delete;
  Scene &operator=(const Scene &) = delete;
  void load(const char *path);
  void render(const Camera &camera, const RenderFunc &render,
              std::chrono::milliseconds time);
  void traverse(const EnterFunc &enter, const LeaveFunc &leave,
                Node *node = nullptr, const DirectX::XMFLOAT4X4 &parent = {});
};
