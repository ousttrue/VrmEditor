#include "vrm/node.h"
#include <iostream>

Node::Node(uint32_t i, std::string_view name) : index(i), name(name) {}

void Node::init() {
  worldInit = world;
  auto t =
      DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);
  auto r =
      DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation));
  auto s = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
  DirectX::XMStoreFloat4x4(&localInit, s * r * t);
}

void Node::addChild(const std::shared_ptr<Node> &parent,
                    const std::shared_ptr<Node> &child) {
  if (auto current_parent = child->parent.lock()) {
    current_parent->children.remove(child);
  }
  child->parent = parent;
  parent->children.push_back(child);
}

void Node::calcWorld(bool recursive) {
  DirectX::XMFLOAT4X4 parentWorld{
      1, 0, 0, 0, //
      0, 1, 0, 0, //
      0, 0, 1, 0, //
      0, 0, 0, 1, //
  };
  if (auto p = parent.lock()) {
    parentWorld = p->world;
  }

  auto t =
      DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);
  auto r =
      DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation));
  auto s = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
  DirectX::XMStoreFloat4x4(&world,
                           s * r * t * DirectX::XMLoadFloat4x4(&parentWorld));

  assert(!std::isnan(world._41));

  if (recursive) {
    for (auto &child : children) {
      child->calcWorld(true);
    }
  }
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

bool Node::setWorldMatrix(const DirectX::XMFLOAT4X4 &world) {

  DirectX::XMMATRIX parentMatrix;
  if (auto parentNode = parent.lock()) {
    parentMatrix = DirectX::XMLoadFloat4x4(&parentNode->world);
  } else {
    parentMatrix = DirectX::XMMatrixIdentity();
  }

  auto inv = DirectX::XMMatrixInverse(nullptr, parentMatrix);
  auto local = DirectX::XMLoadFloat4x4(&world) * inv;

  DirectX::XMFLOAT4X4 m;
  DirectX::XMStoreFloat4x4(&m, local);
  return setLocalMatrix(m);
}

void Node::setWorldRotation(const DirectX::XMFLOAT4 &world, bool recursive) {
  auto parent = parentWorldRotation();
  DirectX::XMStoreFloat4(&rotation, DirectX::XMQuaternionMultiply(
                                        DirectX::XMLoadFloat4(&world),
                                        DirectX::XMQuaternionInverse(
                                            DirectX::XMLoadFloat4(&parent))));
  calcWorld(recursive);
}

void Node::setWorldRotation(const DirectX::XMFLOAT4X4 &world, bool recursive) {

  DirectX::XMMATRIX parentMatrix;
  if (auto parentNode = parent.lock()) {
    parentMatrix = DirectX::XMLoadFloat4x4(&parentNode->world);
  } else {
    parentMatrix = DirectX::XMMatrixIdentity();
  }

  auto inv = DirectX::XMMatrixInverse(nullptr, parentMatrix);
  auto local = DirectX::XMLoadFloat4x4(&world) * inv;

  DirectX::XMVECTOR s;
  DirectX::XMVECTOR r;
  DirectX::XMVECTOR t;
  DirectX::XMMatrixDecompose(&s, &r, &t, local);

  DirectX::XMStoreFloat4(&rotation, r);

  calcWorld(recursive);
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
