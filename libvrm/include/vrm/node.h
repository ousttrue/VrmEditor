#pragma once
#include "humanbones.h"
#include "scenetypes.h"
#include <DirectXMath.h>
#include <assert.h>
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
struct Node;
struct Mesh;

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

enum class NodeConstraintAimAxis
{
  PositiveX,
  NegativeX,
  PositiveY,
  NegativeY,
  PositiveZ,
  NegativeZ,
};
inline NodeConstraintAimAxis
NodeConstraintAimAxisFromName(std::string_view axis)
{
  // "PositiveX", "NegativeX", "PositiveY", "NegativeY", "PositiveZ", "NegativeZ
  if (axis == "PositiveX") {
    return NodeConstraintAimAxis::PositiveX;
  } else if (axis == "NegativeX") {
    return NodeConstraintAimAxis::NegativeX;
  } else if (axis == "PositiveY") {
    return NodeConstraintAimAxis::PositiveY;
  } else if (axis == "NegativeY") {
    return NodeConstraintAimAxis::NegativeY;
  } else if (axis == "PositiveZ") {
    return NodeConstraintAimAxis::PositiveZ;
  } else if (axis == "NegativeZ") {
    return NodeConstraintAimAxis::NegativeZ;
  } else {
    assert(false);
    return {};
  }
}
inline DirectX::XMVECTOR
GetAxisVector(NodeConstraintAimAxis axis)
{
  static DirectX::XMFLOAT3 s_positiveX{ 1, 0, 0 };
  static DirectX::XMFLOAT3 s_negativeX{ -1, 0, 0 };
  static DirectX::XMFLOAT3 s_positiveY{ 0, 1, 0 };
  static DirectX::XMFLOAT3 s_negativeY{ 0, -1, 0 };
  static DirectX::XMFLOAT3 s_positiveZ{ 0, 0, 1 };
  static DirectX::XMFLOAT3 s_negativeZ{ 0, 0, -1 };
  switch (axis) {
    case NodeConstraintAimAxis::PositiveX:
      return DirectX::XMLoadFloat3(&s_positiveX);
    case NodeConstraintAimAxis::NegativeX:
      return DirectX::XMLoadFloat3(&s_negativeX);
    case NodeConstraintAimAxis::PositiveY:
      return DirectX::XMLoadFloat3(&s_positiveY);
    case NodeConstraintAimAxis::NegativeY:
      return DirectX::XMLoadFloat3(&s_negativeY);
    case NodeConstraintAimAxis::PositiveZ:
      return DirectX::XMLoadFloat3(&s_positiveZ);
    case NodeConstraintAimAxis::NegativeZ:
      return DirectX::XMLoadFloat3(&s_negativeZ);
  }
  assert(false);
  return {};
}

enum class NodeConstraintRollAxis
{
  X,
  Y,
  Z,
};
inline NodeConstraintRollAxis
NodeConstraintRollAxisFromName(std::string_view axis)
{
  // "X", "Y", "Z"
  if (axis == "X") {
    return NodeConstraintRollAxis::X;
  } else if (axis == "Y") {
    return NodeConstraintRollAxis::Y;
  } else if (axis == "Z") {
    return NodeConstraintRollAxis::Z;
  } else {
    assert(false);
    return {};
  }
}
inline DirectX::XMVECTOR
GetRollVector(NodeConstraintRollAxis axis)
{
  static DirectX::XMFLOAT3 s_x{ 1, 0, 0 };
  static DirectX::XMFLOAT3 s_y{ 0, 1, 0 };
  static DirectX::XMFLOAT3 s_z{ 0, 0, 1 };
  switch (axis) {
    case NodeConstraintRollAxis::X:
      return DirectX::XMLoadFloat3(&s_x);
    case NodeConstraintRollAxis::Y:
      return DirectX::XMLoadFloat3(&s_y);
    case NodeConstraintRollAxis::Z:
      return DirectX::XMLoadFloat3(&s_z);
    default:
      return {};
  }
}

struct NodeConstraint
{
  NodeConstraintTypes Type;
  std::weak_ptr<Node> Source;
  float Weight = 1.0f;
  union
  {
    NodeConstraintAimAxis AimAxis;
    NodeConstraintRollAxis RollAxis;
  };
  void Process(const std::shared_ptr<Node>& dst);
};

struct EuclideanTransform
{
  DirectX::XMFLOAT4 Rotation = { 0, 0, 0, 1 };
  DirectX::XMFLOAT3 Translation = { 0, 0, 0 };

  bool HasRotation() const
  {
    if (Rotation.x == 0 && Rotation.y == 0 && Rotation.z == 0 &&
        (Rotation.w == 1 || Rotation.w == -1)) {
      return false;
    }
    return true;
  }

  DirectX::XMMATRIX ScalingTranslationMatrix(float scaling) const
  {
    auto r =
      DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&Rotation));
    auto t = DirectX::XMMatrixTranslation(Translation.x * scaling,
                                          Translation.y * scaling,
                                          Translation.z * scaling);
    return r * t;
  }

  DirectX::XMMATRIX Matrix() const
  {
    auto r =
      DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&Rotation));
    auto t =
      DirectX::XMMatrixTranslation(Translation.x, Translation.y, Translation.z);
    return DirectX::XMMatrixMultiply(r, t);
  }
};

struct Node
{
  // uint32_t Index;
  std::string Name;
  std::optional<vrm::HumanBones> Humanoid;
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
  DirectX::XMVECTOR WorldTransformPoint(const DirectX::XMVECTOR& p)
  {
    return DirectX::XMVector3Transform(p, WorldMatrix());
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
  DirectX::XMVECTOR WorldInitialTransformPoint(const DirectX::XMVECTOR& p)
  {
    return DirectX::XMVector3Transform(p, WorldInitialMatrix());
  }

  std::list<std::shared_ptr<Node>> Children;
  std::weak_ptr<Node> Parent;
  std::shared_ptr<Mesh> Mesh;
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
  void SetWorldRotation(const DirectX::XMVECTOR& world, bool recursive = false);
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

}
}
