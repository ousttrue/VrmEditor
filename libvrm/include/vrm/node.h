#pragma once
#include "mesh.h"
#include "scenetypes.h"
#include <DirectXMath.h>
#include <list>
#include <memory>
#include <optional>
#include <span>
#include <stdint.h>
#include <string>
#include <string_view>

struct Skin;
struct MeshInstance;
struct Node {
  uint32_t index;
  std::string name;
  DirectX::XMFLOAT3 translation = {};
  DirectX::XMFLOAT4 rotation = {};
  DirectX::XMFLOAT3 scale = {};
  bool hasRotation() const {
    if (rotation.x == 0 && rotation.y == 0 && rotation.z == 0 &&
        rotation.w == 1) {
      return false;
    }
    return true;
  }
  DirectX::XMFLOAT4X4 localInit;

  DirectX::XMFLOAT4X4 world;
  DirectX::XMFLOAT4X4 worldInit;
  void init();

  std::optional<uint32_t> mesh;
  std::shared_ptr<MeshInstance> Instance;

  std::shared_ptr<Skin> skin;

  std::list<std::shared_ptr<Node>> children;
  std::weak_ptr<Node> parent;

  Node(uint32_t i, std::string_view name);
  Node(const Node &) = delete;
  Node &operator=(const Node &) = delete;
  static void addChild(const std::shared_ptr<Node> &parent,
                       const std::shared_ptr<Node> &child);
  void calcWorld(bool recursive = false);
  bool setLocalMatrix(const DirectX::XMFLOAT4X4 &local);
  bool setWorldMatrix(const DirectX::XMFLOAT4X4 &world);
  DirectX::XMFLOAT3 worldPosition() const {
    return {world._41, world._42, world._43};
  }
  void setWorldRotation(const DirectX::XMFLOAT4 &world, bool recursive = false);
  DirectX::XMFLOAT4 worldRotation() const {
    auto q =
        DirectX::XMQuaternionRotationMatrix(DirectX::XMLoadFloat4x4(&world));
    DirectX::XMFLOAT4 tmp;
    DirectX::XMStoreFloat4(&tmp, q);
    return tmp;
  }
  DirectX::XMFLOAT4X4 parentWorld() const {
    if (auto p = parent.lock()) {
      return p->world;
    } else {
      return {
          1, 0, 0, 0, //
          0, 1, 0, 0, //
          0, 0, 1, 0, //
          0, 0, 0, 1, //
      };
    }
  }
  DirectX::XMFLOAT4 parentWorldRotation() const {
    if (auto p = parent.lock()) {
      return p->worldRotation();
    } else {
      return {0, 0, 0, 1};
    }
  }
  DirectX::XMFLOAT3 parentWorldPosition() const {
    if (auto p = parent.lock()) {
      return p->worldPosition();
    } else {
      return {0, 0, 0};
    }
  }
  void print(int level = 0);
  void setWorldRotation(const DirectX::XMFLOAT4X4 &world,
                        bool recursive = false);
};
inline std::ostream &operator<<(std::ostream &os, const Node &node) {
  os << "Node[" << node.index << "]" << node.name
      // << ": " << node.translation
      // << node.rotation
      // << node.scale
      ;
  if (node.mesh) {
    os << ", mesh: " << *node.mesh;
  }
  return os;
}
