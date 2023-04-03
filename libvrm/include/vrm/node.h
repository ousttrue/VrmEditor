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

namespace gltf {
struct Skin;
struct MeshInstance;
struct Node {
  uint32_t Index;
  std::string Name;
  EuclideanTransform Transform;
  DirectX::XMFLOAT3 Translation()const
  {
    return Transform.Translation;
  }
  DirectX::XMFLOAT4 Rotation()const
  {
    return Transform.Rotation;
  }
  DirectX::XMFLOAT3 Scale = {};
  DirectX::XMFLOAT4X4 WorldMatrix;

  DirectX::XMFLOAT4X4 LocalInitialMatrix;
  DirectX::XMFLOAT4X4 WorldInitialMatrix;

  std::list<std::shared_ptr<Node>> Children;
  std::weak_ptr<Node> Parent;

  std::optional<uint32_t> Mesh;
  std::shared_ptr<MeshInstance> Instance;

  std::shared_ptr<Skin> Skin;

  Node(uint32_t i, std::string_view name);
  Node(const Node &) = delete;
  Node &operator=(const Node &) = delete;

  static void AddChild(const std::shared_ptr<Node> &parent,
                       const std::shared_ptr<Node> &child);

  void CalcInitialMatrix();

  bool SetLocalMatrix(const DirectX::XMFLOAT4X4 &local);

  void CalcWorldMatrix(bool recursive = false);
  bool SetWorldMatrix(const DirectX::XMFLOAT4X4 &world);
  DirectX::XMFLOAT3 WorldPosition() const {
    return {WorldMatrix._41, WorldMatrix._42, WorldMatrix._43};
  }
  void SetWorldRotation(const DirectX::XMFLOAT4 &world, bool recursive = false);
  void SetWorldRotation(const DirectX::XMFLOAT4X4 &world, bool recursive = false);
  DirectX::XMFLOAT4 worldRotation() const {
    auto q =
        DirectX::XMQuaternionRotationMatrix(DirectX::XMLoadFloat4x4(&WorldMatrix));
    DirectX::XMFLOAT4 tmp;
    DirectX::XMStoreFloat4(&tmp, q);
    return tmp;
  }

  DirectX::XMFLOAT4X4 ParentWorld() const {
    if (auto p = Parent.lock()) {
      return p->WorldMatrix;
    } else {
      return {
          1, 0, 0, 0, //
          0, 1, 0, 0, //
          0, 0, 1, 0, //
          0, 0, 0, 1, //
      };
    }
  }
  DirectX::XMFLOAT4 ParentWorldRotation() const {
    if (auto p = Parent.lock()) {
      return p->worldRotation();
    } else {
      return {0, 0, 0, 1};
    }
  }
  DirectX::XMFLOAT3 ParentWorldPosition() const {
    if (auto p = Parent.lock()) {
      return p->WorldPosition();
    } else {
      return {0, 0, 0};
    }
  }
  void Print(int level = 0);
};
inline std::ostream &operator<<(std::ostream &os, const Node &node) {
  os << "Node[" << node.Index << "]" << node.Name
      // << ": " << node.translation
      // << node.rotation
      // << node.scale
      ;
  if (node.Mesh) {
    os << ", mesh: " << *node.Mesh;
  }
  return os;
}
} // namespace gltf
