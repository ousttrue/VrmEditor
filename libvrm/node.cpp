#include "vrm/node.h"
#include <iostream>

Node::Node(uint32_t i, std::string_view name) : index(i), name(name) {}

void Node::addChild(const std::shared_ptr<Node> &child) {
  if (auto current_parent = child->parent.lock()) {
    current_parent->children.remove(child);
  }
  child->parent = shared_from_this();
  children.push_back(child);
}

void Node::calcWorld(const DirectX::XMFLOAT4X4 &parent) {
  auto t =
      DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);
  auto r = DirectX::XMMatrixRotationQuaternion(
      DirectX::XMLoadFloat4((DirectX::XMFLOAT4 *)&rotation));
  auto s = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
  DirectX::XMStoreFloat4x4(&world,
                           s * r * t * DirectX::XMLoadFloat4x4(&parent));
}

bool Node::setLocalMatrix(const DirectX::XMFLOAT4X4 &local) {
  DirectX::XMVECTOR s;
  DirectX::XMVECTOR r;
  DirectX::XMVECTOR t;
  if (!DirectX::XMMatrixDecompose(&s, &r, &t,
                                  DirectX::XMLoadFloat4x4(&local))) {
    return false;
  }
  DirectX::XMStoreFloat3((DirectX::XMFLOAT3 *)&scale, s);
  DirectX::XMStoreFloat4((DirectX::XMFLOAT4 *)&rotation, r);
  DirectX::XMStoreFloat3((DirectX::XMFLOAT3 *)&translation, t);
  return true;
}

bool Node::setWorldMatrix(const DirectX::XMFLOAT4X4 &world,
                          const DirectX::XMFLOAT4X4 &parent) {
  auto inv = DirectX::XMMatrixInverse(
      nullptr, DirectX::XMLoadFloat4x4((DirectX::XMFLOAT4X4 *)&parent));
  auto local = DirectX::XMLoadFloat4x4(&world) * inv;

  DirectX::XMFLOAT4X4 m;
  DirectX::XMStoreFloat4x4(&m, local);
  return setLocalMatrix(m);
}

void Node::print(int level) {
  for (int i = 0; i < level; ++i) {
    std::cout << "  ";
  }
  std::cout << name << std::endl;
  for (auto child : children) {
    child->print(level + 1);
  }
}
