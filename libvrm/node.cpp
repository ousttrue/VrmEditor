#include "vrm/node.h"
#include <iostream>

namespace gltf {
Node::Node(std::string_view name)
  : Name(name)
{
}

void
Node::CalcInitialMatrix()
{
  WorldInitialTransform = WorldTransform;
  WorldInitialScale = WorldScale;
  InitialTransform = Transform;
  InitialScale = Scale;
}

void
Node::AddChild(const std::shared_ptr<Node>& parent,
               const std::shared_ptr<Node>& child)
{
  if (auto current_parent = child->Parent.lock()) {
    current_parent->Children.remove(child);
  }
  child->Parent = parent;
  parent->Children.push_back(child);
}

void
Node::CalcWorldMatrix(bool recursive)
{
  auto world = Matrix() * ParentWorldMatrix();

  DirectX::XMVECTOR s;
  DirectX::XMVECTOR r;
  DirectX::XMVECTOR t;
  assert(DirectX::XMMatrixDecompose(&s, &r, &t, world));
  DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&WorldScale, s);
  DirectX::XMStoreFloat4((DirectX::XMFLOAT4*)&WorldTransform.Rotation, r);
  DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&WorldTransform.Translation, t);

  if (recursive) {
    for (auto& child : Children) {
      child->CalcWorldMatrix(true);
    }
  }
}

bool
Node::SetLocalMatrix(const DirectX::XMMATRIX& local)
{
  DirectX::XMVECTOR s;
  DirectX::XMVECTOR r;
  DirectX::XMVECTOR t;
  if (!DirectX::XMMatrixDecompose(&s, &r, &t, local)) {
    return false;
  }
  DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&Scale, s);
  DirectX::XMStoreFloat4((DirectX::XMFLOAT4*)&Transform.Rotation, r);
  DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&Transform.Translation, t);
  return true;
}

bool
Node::SetWorldMatrix(const DirectX::XMMATRIX& world)
{
  auto parentMatrix = ParentWorldMatrix();
  auto inv = DirectX::XMMatrixInverse(nullptr, parentMatrix);
  auto local = world * inv;
  return SetLocalMatrix(local);
}

void
Node::SetWorldRotation(const DirectX::XMFLOAT4& world, bool recursive)
{
  auto parent = ParentWorldRotation();
  DirectX::XMStoreFloat4(
    &Transform.Rotation,
    DirectX::XMQuaternionMultiply(
      DirectX::XMLoadFloat4(&world),
      DirectX::XMQuaternionInverse(DirectX::XMLoadFloat4(&parent))));
  CalcWorldMatrix(recursive);
}

void
Node::SetWorldRotation(const DirectX::XMFLOAT4X4& world, bool recursive)
{
  auto parentMatrix = ParentWorldMatrix();
  auto inv = DirectX::XMMatrixInverse(nullptr, parentMatrix);
  auto local = DirectX::XMLoadFloat4x4(&world) * inv;

  DirectX::XMVECTOR s;
  DirectX::XMVECTOR r;
  DirectX::XMVECTOR t;
  DirectX::XMMatrixDecompose(&s, &r, &t, local);

  DirectX::XMStoreFloat4(&Transform.Rotation, r);

  CalcWorldMatrix(recursive);
}

void
Node::Print(int level)
{
  for (int i = 0; i < level; ++i) {
    std::cout << "  ";
  }
  std::cout << Name << std::endl;
  for (auto child : Children) {
    child->Print(level + 1);
  }
}
} // namespace gltf
