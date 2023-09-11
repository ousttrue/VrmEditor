#pragma once
#include "constraint.h"
#include "humanoid/humanbones.h"
#include <DirectXMath.h>
#include <assert.h>
#include <functional>
#include <grapho/euclidean_transform.h>
#include <iostream>
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

struct Node
{
  // uint32_t Index;
  std::string Name;
  const char* GetLabel() const { return Name.c_str(); }

  std::optional<HumanBones> Humanoid;
  std::optional<HumanBones> GetHumanBone() const { return Humanoid; }
  void SetHumanBone(std::optional<HumanBones> bone) { Humanoid = bone; }

  std::list<std::shared_ptr<Node>> Children;
  std::weak_ptr<Node> Parent;
  static void AddChild(const std::shared_ptr<Node>& parent,
                       const std::shared_ptr<Node>& child);

  // initial local
  grapho::EuclideanTransform InitialTransform = {};
  DirectX::XMFLOAT3 InitialScale = { 1, 1, 1 };
  DirectX::XMMATRIX InitialMatrix() const
  {
    return DirectX::XMMatrixMultiply(
      DirectX::XMMatrixScaling(InitialScale.x, InitialScale.y, InitialScale.z),
      InitialTransform.Matrix());
  }
  bool SetLocalInitialMatrix(const DirectX::XMMATRIX& local)
  {
    DirectX::XMVECTOR s;
    DirectX::XMVECTOR r;
    DirectX::XMVECTOR t;
    if (!DirectX::XMMatrixDecompose(&s, &r, &t, local)) {
      return false;
    }
    DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&InitialScale, s);
    DirectX::XMStoreFloat4((DirectX::XMFLOAT4*)&InitialTransform.Rotation, r);
    DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&InitialTransform.Translation,
                           t);
    return true;
  }
  DirectX::XMFLOAT3& GetTranslation() { return InitialTransform.Translation; }
  DirectX::XMFLOAT4& GetRotation() { return InitialTransform.Rotation; }
  DirectX::XMFLOAT3& GetScale() { return InitialScale; }
  void Calc(bool rec) { CalcWorldInitialMatrix(rec); }

  // initial world
  grapho::EuclideanTransform WorldInitialTransform = {};
  DirectX::XMFLOAT3 WorldInitialScale = { 1, 1, 1 };
  DirectX::XMMATRIX WorldInitialMatrix() const
  {
    return DirectX::XMMatrixMultiply(
      DirectX::XMMatrixScaling(
        WorldInitialScale.x, WorldInitialScale.y, WorldInitialScale.z),
      WorldInitialTransform.Matrix());
  }
  DirectX::XMMATRIX ParentWorldInitialMatrix() const
  {
    if (auto p = Parent.lock()) {
      return p->WorldInitialMatrix();
    } else {
      return DirectX::XMMatrixIdentity();
    }
  }
  bool SetWorldInitialMatrix(const DirectX::XMMATRIX& world)
  {
    DirectX::XMVECTOR s;
    DirectX::XMVECTOR r;
    DirectX::XMVECTOR t;
    if (!DirectX::XMMatrixDecompose(&s, &r, &t, world)) {
      return false;
    }
    DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&WorldInitialScale, s);
    DirectX::XMStoreFloat4((DirectX::XMFLOAT4*)&WorldInitialTransform.Rotation,
                           r);
    DirectX::XMStoreFloat3(
      (DirectX::XMFLOAT3*)&WorldInitialTransform.Translation, t);
    auto parentMatrix = ParentWorldInitialMatrix();
    auto inv = DirectX::XMMatrixInverse(nullptr, parentMatrix);
    auto local = world * inv;
    return SetLocalInitialMatrix(local);
  }

  void CalcWorldInitialMatrix(bool recursive = false)
  {
    auto world = InitialMatrix() * ParentWorldInitialMatrix();

    DirectX::XMVECTOR s;
    DirectX::XMVECTOR r;
    DirectX::XMVECTOR t;
    if (!DirectX::XMMatrixDecompose(&s, &r, &t, world)) {
      std::cerr << "XMMatrixDecompose" << std::endl;
      return;
    }
    DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&WorldInitialScale, s);
    DirectX::XMStoreFloat4((DirectX::XMFLOAT4*)&WorldInitialTransform.Rotation,
                           r);
    DirectX::XMStoreFloat3(
      (DirectX::XMFLOAT3*)&WorldInitialTransform.Translation, t);

    if (recursive) {
      for (auto& child : Children) {
        child->CalcWorldInitialMatrix(true);
      }
    }
  }

  DirectX::XMVECTOR WorldInitialTransformPoint(const DirectX::XMVECTOR& p)
  {
    return DirectX::XMVector3Transform(p, WorldInitialMatrix());
  }
  DirectX::XMFLOAT3 ParentWorldInitialPosition() const
  {
    if (auto p = Parent.lock()) {
      return p->WorldInitialTransform.Translation;
    } else {
      return { 0, 0, 0 };
    }
  }

  Node(std::string_view name);
  Node(std::u8string_view name)
    : Node(std::string_view{ (const char*)name.data(),
                             (const char*)name.data() + name.size() })
  {
  }
  Node(const Node&) = delete;
  Node& operator=(const Node&) = delete;

  DirectX::XMFLOAT4X4 ShapeMatrix;
  DirectX::XMFLOAT4 ShapeColor = { 1, 1, 1, 1 };
  void CalcShape();
  std::optional<HumanBones> ClosestBone();
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
  std::shared_ptr<Node> GetShapeTail();
};

} // namespace
