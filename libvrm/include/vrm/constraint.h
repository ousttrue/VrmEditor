#pragma once
#include <DirectXMath.h>
#include <assert.h>
#include <memory>
#include <string_view>

namespace libvrm {
namespace gltf {

struct Node;
}

namespace vrm {

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
  std::weak_ptr<gltf::Node> Source;
  float Weight = 1.0f;
  union
  {
    NodeConstraintAimAxis AimAxis;
    NodeConstraintRollAxis RollAxis;
  };
  void Process(const std::shared_ptr<gltf::Node>& dst);
};

}
}
