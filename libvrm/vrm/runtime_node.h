#pragma once
#include "constraint.h"
#include <functional>
#include <grapho/euclidean_transform.h>
#include <vrm/node.h>

namespace libvrm {

struct Instance
{
  DirectX::XMFLOAT4X4 Matrix;
  DirectX::XMFLOAT4 Color;
};
using PushInstance = std::function<void(const Instance&)>;

struct RuntimeNode
{
  std::shared_ptr<Node> Base;
  const char* GetLabel() const { return Base->GetLabel(); }
  std::optional<HumanBones> GetHumanBone() const
  {
    return Base->GetHumanBone();
  }
  void SetHumanBone(std::optional<HumanBones> bone) { Base->Humanoid = bone; }

  std::optional<NodeConstraint> Constraint;

  RuntimeNode(const std::shared_ptr<Node>& node)
    : Base(node)
  {
    Transform = node->InitialTransform;
    Scale = node->InitialScale;
    WorldTransform = node->WorldInitialTransform;
    WorldScale = node->WorldInitialScale;
  }

  std::list<std::shared_ptr<RuntimeNode>> Children;
  std::weak_ptr<RuntimeNode> Parent;
  static void AddChild(const std::shared_ptr<RuntimeNode>& parent,
                       const std::shared_ptr<RuntimeNode>& child)
  {
    if (auto current_parent = child->Parent.lock()) {
      current_parent->Children.remove(child);
    }
    child->Parent = parent;
    parent->Children.push_back(child);
  }

  // local
  grapho::EuclideanTransform Transform = {};
  DirectX::XMFLOAT3 Scale = { 1, 1, 1 };
  DirectX::XMMATRIX Matrix() const
  {
    return DirectX::XMMatrixMultiply(
      DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z), Transform.Matrix());
  }
  DirectX::XMFLOAT3& GetTranslation() { return Transform.Translation; }
  DirectX::XMFLOAT4& GetRotation() { return Transform.Rotation; }
  DirectX::XMFLOAT3& GetScale() { return Scale; }
  void Calc(bool rec) { CalcWorldMatrix(rec); }

  // world
  grapho::EuclideanTransform WorldTransform = {};
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

  void CalcWorldMatrix(bool recursive = false)
  {
    auto world = Matrix() * ParentWorldMatrix();

    DirectX::XMVECTOR s;
    DirectX::XMVECTOR r;
    DirectX::XMVECTOR t;
    if (DirectX::XMMatrixDecompose(&s, &r, &t, world)) {
      DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&WorldScale, s);
      DirectX::XMStoreFloat4((DirectX::XMFLOAT4*)&WorldTransform.Rotation, r);
      DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&WorldTransform.Translation,
                             t);
    }

    if (recursive) {
      for (auto& child : Children) {
        child->CalcWorldMatrix(true);
      }
    }
  }

  bool SetLocalMatrix(const DirectX::XMMATRIX& local)
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

  bool SetWorldMatrix(const DirectX::XMMATRIX& world)
  {
    DirectX::XMVECTOR s;
    DirectX::XMVECTOR r;
    DirectX::XMVECTOR t;
    if (!DirectX::XMMatrixDecompose(&s, &r, &t, world)) {
      return false;
    }
    DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&WorldScale, s);
    DirectX::XMStoreFloat4((DirectX::XMFLOAT4*)&WorldTransform.Rotation, r);
    DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&WorldTransform.Translation, t);
    auto parentMatrix = ParentWorldMatrix();
    auto inv = DirectX::XMMatrixInverse(nullptr, parentMatrix);
    auto local = world * inv;
    return SetLocalMatrix(local);
  }

  void SetWorldRotation(const DirectX::XMVECTOR& world, bool recursive = false)
  {
    auto parent = ParentWorldRotation();
    DirectX::XMStoreFloat4(&Transform.Rotation,
                           DirectX::XMQuaternionMultiply(
                             world, DirectX::XMQuaternionInverse(parent)));
    CalcWorldMatrix(recursive);
  }

  void SetWorldRotation(const DirectX::XMFLOAT4X4& world,
                        bool recursive = false)
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

  void UpdateShapeInstanceRecursive(DirectX::XMMATRIX parent,
                                    const PushInstance& pushInstance)
  {
    auto m = Transform.Matrix() * parent;
    auto shape = DirectX::XMLoadFloat4x4(&Base->ShapeMatrix);

    Instance instance;
    DirectX::XMStoreFloat4x4(&instance.Matrix, shape * m);
    // instance.Color = ShapeColor;
    pushInstance(instance);
    for (auto& child : Children) {
      child->UpdateShapeInstanceRecursive(m, pushInstance);
    }
  }
};

} // namespace
