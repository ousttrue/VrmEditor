#include "vrm/node.h"
#include "vrm/dmath.h"
#include <iostream>

namespace gltf {
Node::Node(uint32_t i, std::string_view name)
  : Index(i)
  , Name(name)
{
}

void
Node::CalcInitialMatrix()
{
  WorldInitialMatrix = WorldMatrix;
  LocalInitialMatrix = dmath::trs(Translation(), Rotation(), Scale);
}

void
Node::addChild(const std::shared_ptr<Node>& parent,
               const std::shared_ptr<Node>& child)
{
  if (auto current_parent = child->Parent.lock()) {
    current_parent->Children.remove(child);
  }
  child->Parent = parent;
  parent->Children.push_back(child);
}

void
Node::calcWorld(bool recursive)
{
  DirectX::XMFLOAT4X4 parentWorld{
    1, 0, 0, 0, //
    0, 1, 0, 0, //
    0, 0, 1, 0, //
    0, 0, 0, 1, //
  };
  if (auto p = Parent.lock()) {
    parentWorld = p->WorldMatrix;
  }

  auto t = DirectX::XMMatrixTranslation(
    Transform.Translation.x, Transform.Translation.y, Transform.Translation.z);
  auto r =
    DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&Transform.Rotation));
  auto s = DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z);
  DirectX::XMStoreFloat4x4(&WorldMatrix,
                           s * r * t * DirectX::XMLoadFloat4x4(&parentWorld));

  assert(!std::isnan(WorldMatrix._41));

  if (recursive) {
    for (auto& child : Children) {
      child->calcWorld(true);
    }
  }
}

bool
Node::setLocalMatrix(const DirectX::XMFLOAT4X4& local)
{
  DirectX::XMVECTOR s;
  DirectX::XMVECTOR r;
  DirectX::XMVECTOR t;
  if (!DirectX::XMMatrixDecompose(
        &s, &r, &t, DirectX::XMLoadFloat4x4(&local))) {
    return false;
  }
  DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&Scale, s);
  DirectX::XMStoreFloat4((DirectX::XMFLOAT4*)&Transform.Rotation, r);
  DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&Transform.Translation, t);
  return true;
}

bool
Node::setWorldMatrix(const DirectX::XMFLOAT4X4& world)
{

  DirectX::XMMATRIX parentMatrix;
  if (auto parentNode = Parent.lock()) {
    parentMatrix = DirectX::XMLoadFloat4x4(&parentNode->WorldMatrix);
  } else {
    parentMatrix = DirectX::XMMatrixIdentity();
  }

  auto inv = DirectX::XMMatrixInverse(nullptr, parentMatrix);
  auto local = DirectX::XMLoadFloat4x4(&world) * inv;

  DirectX::XMFLOAT4X4 m;
  DirectX::XMStoreFloat4x4(&m, local);
  return setLocalMatrix(m);
}

void
Node::setWorldRotation(const DirectX::XMFLOAT4& world, bool recursive)
{
  auto parent = parentWorldRotation();
  DirectX::XMStoreFloat4(
    &Transform.Rotation,
    DirectX::XMQuaternionMultiply(
      DirectX::XMLoadFloat4(&world),
      DirectX::XMQuaternionInverse(DirectX::XMLoadFloat4(&parent))));
  calcWorld(recursive);
}

void
Node::setWorldRotation(const DirectX::XMFLOAT4X4& world, bool recursive)
{

  DirectX::XMMATRIX parentMatrix;
  if (auto parentNode = Parent.lock()) {
    parentMatrix = DirectX::XMLoadFloat4x4(&parentNode->WorldMatrix);
  } else {
    parentMatrix = DirectX::XMMatrixIdentity();
  }

  auto inv = DirectX::XMMatrixInverse(nullptr, parentMatrix);
  auto local = DirectX::XMLoadFloat4x4(&world) * inv;

  DirectX::XMVECTOR s;
  DirectX::XMVECTOR r;
  DirectX::XMVECTOR t;
  DirectX::XMMatrixDecompose(&s, &r, &t, local);

  DirectX::XMStoreFloat4(&Transform.Rotation, r);

  calcWorld(recursive);
}

void
Node::print(int level)
{
  for (int i = 0; i < level; ++i) {
    std::cout << "  ";
  }
  std::cout << Name << std::endl;
  for (auto child : Children) {
    child->print(level + 1);
  }
}
} // namespace gltf
