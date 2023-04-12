#pragma once
#include "humanbones.h"
#include "scenetypes.h"
#include <DirectXMath.h>
#include <list>
#include <memory>
#include <optional>
#include <span>
#include <sstream>
#include <stdint.h>
#include <string>
#include <string_view>
#include <vector>

namespace gltf {

struct Skin;
struct MeshInstance;
struct Scene;
struct Node;

struct NodeHumanoidInfo {
  vrm::HumanBones HumanBone;
  std::weak_ptr<Node> Parent;
  std::list<std::shared_ptr<Node>> Children;
};

struct Node {
  // uint32_t Index;
  std::string Name;
  std::optional<NodeHumanoidInfo> Humanoid;

  // local
  EuclideanTransform Transform = {};
  DirectX::XMFLOAT3 Scale = {1, 1, 1};
  DirectX::XMMATRIX Matrix() const {
    return DirectX::XMMatrixMultiply(
        DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z),
        Transform.Matrix());
  }

  // world
  EuclideanTransform WorldTransform = {};
  DirectX::XMFLOAT3 WorldScale = {1, 1, 1};
  DirectX::XMMATRIX WorldMatrix() const {
    return DirectX::XMMatrixMultiply(
        DirectX::XMMatrixScaling(WorldScale.x, WorldScale.y, WorldScale.z),
        WorldTransform.Matrix());
  }

  // initial local
  EuclideanTransform InitialTransform = {};
  DirectX::XMFLOAT3 InitialScale = {1, 1, 1};
  DirectX::XMMATRIX InitialMatrix() const {
    return DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(InitialScale.x,
                                                              InitialScale.y,
                                                              InitialScale.z),
                                     InitialTransform.Matrix());
  }

  // initial world
  EuclideanTransform WorldInitialTransform = {};
  DirectX::XMFLOAT3 WorldInitialScale = {1, 1, 1};
  DirectX::XMMATRIX WorldInitialMatrix() const {
    return DirectX::XMMatrixMultiply(
        DirectX::XMMatrixScaling(WorldInitialScale.x, WorldInitialScale.y,
                                 WorldInitialScale.z),
        WorldInitialTransform.Matrix());
  }

  std::list<std::shared_ptr<Node>> Children;
  std::weak_ptr<Node> Parent;

  std::optional<uint32_t> Mesh;
  std::shared_ptr<MeshInstance> Instance;

  std::shared_ptr<Skin> Skin;

  DirectX::XMFLOAT4X4 ShapeMatrix;

  Node(std::string_view name);
  Node(const Node &) = delete;
  Node &operator=(const Node &) = delete;

  static void AddChild(const std::shared_ptr<Node> &parent,
                       const std::shared_ptr<Node> &child);

  void CalcInitialMatrix();

  bool SetLocalMatrix(const DirectX::XMMATRIX &local);

  void CalcWorldMatrix(bool recursive = false);
  bool SetWorldMatrix(const DirectX::XMMATRIX &world);
  void SetWorldRotation(const DirectX::XMFLOAT4 &world, bool recursive = false);
  void SetWorldRotation(const DirectX::XMFLOAT4X4 &world,
                        bool recursive = false);

  DirectX::XMMATRIX ParentWorldMatrix() const {
    if (auto p = Parent.lock()) {
      return p->WorldMatrix();
    } else {
      return DirectX::XMMatrixIdentity();
    }
  }
  DirectX::XMFLOAT4 ParentWorldRotation() const {
    if (auto p = Parent.lock()) {
      return p->WorldTransform.Rotation;
    } else {
      return {0, 0, 0, 1};
    }
  }
  DirectX::XMFLOAT3 ParentWorldPosition() const {
    if (auto p = Parent.lock()) {
      return p->WorldTransform.Translation;
    } else {
      return {0, 0, 0};
    }
  }
  void Print(int level = 0);
  void CalcShape(int level = 0);
  void UpdateShapeInstanceRecursive(DirectX::XMMATRIX parent,
                                    std::vector<DirectX::XMFLOAT4X4> &out);
};

inline std::ostream &operator<<(std::ostream &os, const Node &node) {
  os << "Node: " << node.Name
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
