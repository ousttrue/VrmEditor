#pragma once
#include <DirectXMath.h>
#include <memory>
#include <vector>

namespace libvrm {

struct RuntimeNode;

enum class SpringColliderShapeType
{
  Sphere,
  Capsule,
};

struct SpringCollider
{
  std::shared_ptr<RuntimeNode> Node;
  SpringColliderShapeType Type = SpringColliderShapeType::Sphere;
  DirectX::XMFLOAT3 Offset = { 0, 0, 0 };
  float Radius = 0;
  DirectX::XMFLOAT3 Tail = { 0, 0, 0 };
};

struct SpringColliderGroup
{
  std::vector<std::shared_ptr<SpringCollider>> Colliders;
};

} // namespace
