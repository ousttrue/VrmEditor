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
  float3 translation = {};
  quaternion rotation = {};
  float3 scale = {};

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
  bool setLocalMatrix(const DirectX::XMFLOAT4X4 &local);
  bool setWorldMatrix(const DirectX::XMFLOAT4X4 &world);
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