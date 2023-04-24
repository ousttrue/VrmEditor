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

namespace libvrm {
namespace gltf {

struct Skin;
struct MeshInstance;
struct Scene;
struct Node;

struct NodeHumanoidInfo
{
  vrm::HumanBones HumanBone;
  std::weak_ptr<Node> Parent;
  std::list<std::shared_ptr<Node>> Children;
};

struct Instance
{
  DirectX::XMFLOAT4X4 Matrix;
  DirectX::XMFLOAT4 Color;
};
using PushInstance = std::function<void(const Instance&)>;

enum class NodeConstraintTypes
{
  Roll,
  Aim,
  Rotation,
};

struct NodeConstraint
{
  NodeConstraintTypes Type;
  std::weak_ptr<Node> Source;
};

struct Node
{
  // uint32_t Index;
  std::string Name;
  std::optional<NodeHumanoidInfo> Humanoid;
  std::optional<NodeConstraint> Constraint;

  // local
  EuclideanTransform Transform = {};
  DirectX::XMFLOAT3 Scale = { 1, 1, 1 };
  DirectX::XMMATRIX Matrix() const
  {
    return DirectX::XMMatrixMultiply(
      DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z), Transform.Matrix());
  }

  // world
  EuclideanTransform WorldTransform = {};
  DirectX::XMFLOAT3 WorldScale = { 1, 1, 1 };
  DirectX::XMMATRIX WorldMatrix() const
  {
    return DirectX::XMMatrixMultiply(
      DirectX::XMMatrixScaling(WorldScale.x, WorldScale.y, WorldScale.z),
      WorldTransform.Matrix());
  }

  // initial local
  EuclideanTransform InitialTransform = {};
  DirectX::XMFLOAT3 InitialScale = { 1, 1, 1 };
  DirectX::XMMATRIX InitialMatrix() const
  {
    return DirectX::XMMatrixMultiply(
      DirectX::XMMatrixScaling(InitialScale.x, InitialScale.y, InitialScale.z),
      InitialTransform.Matrix());
  }

  // initial world
  EuclideanTransform WorldInitialTransform = {};
  DirectX::XMFLOAT3 WorldInitialScale = { 1, 1, 1 };
  DirectX::XMMATRIX WorldInitialMatrix() const
  {
    return DirectX::XMMatrixMultiply(
      DirectX::XMMatrixScaling(
        WorldInitialScale.x, WorldInitialScale.y, WorldInitialScale.z),
      WorldInitialTransform.Matrix());
  }

  std::list<std::shared_ptr<Node>> Children;
  std::weak_ptr<Node> Parent;

  std::optional<uint32_t> Mesh;
  std::shared_ptr<MeshInstance> MeshInstance;

  std::shared_ptr<Skin> Skin;

  DirectX::XMFLOAT4X4 ShapeMatrix;
  DirectX::XMFLOAT4 ShapeColor = { 1, 1, 1, 1 };

  Node(std::string_view name);
  Node(const Node&) = delete;
  Node& operator=(const Node&) = delete;

  static void AddChild(const std::shared_ptr<Node>& parent,
                       const std::shared_ptr<Node>& child);

  void CalcInitialMatrix();

  bool SetLocalMatrix(const DirectX::XMMATRIX& local);

  void CalcWorldMatrix(bool recursive = false);
  bool SetWorldMatrix(const DirectX::XMMATRIX& world);
  void SetWorldRotation(const DirectX::XMFLOAT4& world, bool recursive = false);
  void SetWorldRotation(const DirectX::XMFLOAT4X4& world,
                        bool recursive = false);

  DirectX::XMMATRIX ParentWorldMatrix() const
  {
    if (auto p = Parent.lock()) {
      return p->WorldMatrix();
    } else {
      return DirectX::XMMatrixIdentity();
    }
  }
  DirectX::XMVECTOR ParentWorldRotation() const
  {
    if (auto p = Parent.lock()) {
      return DirectX::XMLoadFloat4(&p->WorldTransform.Rotation);
    } else {
      return DirectX::XMQuaternionIdentity();
    }
  }
  DirectX::XMFLOAT3 ParentWorldPosition() const
  {
    if (auto p = Parent.lock()) {
      return p->WorldTransform.Translation;
    } else {
      return { 0, 0, 0 };
    }
  }
  void Print(int level = 0);
  std::shared_ptr<Node> GetShapeTail();
  std::optional<vrm::HumanBones> ClosestBone();
  bool AnyTail()
  {
    for (auto& child : Children) {
      if (child->Humanoid) {
        return true;
      }
      if (child->AnyTail()) {
        return true;
      }
    }
    return false;
  }
  void CalcShape();
  void UpdateShapeInstanceRecursive(DirectX::XMMATRIX parent,
                                    const PushInstance& pushInstance);
};

inline std::ostream&
operator<<(std::ostream& os, const Node& node)
{
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
}
