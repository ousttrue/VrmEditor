#pragma once
#include "scenetypes.h"
#include <DirectXMath.h>
#include <list>
#include <memory>
#include <optional>
#include <stdint.h>
#include <string>
#include <string_view>

struct Skin;
struct Node : public std::enable_shared_from_this<Node> {
  uint32_t index;
  std::string name;
  DirectX::XMFLOAT3 translation = {};
  DirectX::XMFLOAT4 rotation = {};
  DirectX::XMFLOAT3 scale = {};

  DirectX::XMFLOAT4X4 world;
  DirectX::XMFLOAT4X4 worldInit;

  std::optional<uint32_t> mesh;
  std::shared_ptr<Skin> skin;

  std::list<std::shared_ptr<Node>> children;
  std::weak_ptr<Node> parent;

  Node(uint32_t i, std::string_view name);
  Node(const Node &) = delete;
  Node &operator=(const Node &) = delete;
  void addChild(const std::shared_ptr<Node> &child);
  void calcWorld(const DirectX::XMFLOAT4X4 &parent);
  void calcWorld() {
    if (auto p = parent.lock()) {
      calcWorld(p->world);
    } else {
      DirectX::XMFLOAT4X4 identity{
          1, 0, 0, 0, //
          0, 1, 0, 0, //
          0, 0, 1, 0, //
          0, 0, 0, 1, //
      };
      calcWorld(identity);
    }
  }
  bool setLocalMatrix(const DirectX::XMFLOAT4X4 &local);
  bool setWorldMatrix(const DirectX::XMFLOAT4X4 &world);
  DirectX::XMFLOAT3 worldPosition() const {
    return {world._41, world._42, world._43};
  }
  DirectX::XMFLOAT4 worldRotation() const {
    auto q =
        DirectX::XMQuaternionRotationMatrix(DirectX::XMLoadFloat4x4(&world));
    DirectX::XMFLOAT4 tmp;
    DirectX::XMStoreFloat4(&tmp, q);
    return tmp;
  }
  DirectX::XMFLOAT4 parentWorldRotation() const {
    if (auto p = parent.lock()) {
      return p->worldRotation();
    } else {
      return {0, 0, 0, 1};
    }
  }
  void print(int level = 0);
  void setWorldRotation(const DirectX::XMFLOAT4X4 &world);
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
