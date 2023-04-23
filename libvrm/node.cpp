#include "vrm/node.h"
// #include "vrm/mesh.h"
// #include "vrm/scene.h"
// #include "vrm/vrm0.h"
// #include "vrm/vrm1.h"
#include <iostream>

namespace libvrm::gltf {
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

void
Node::SetWorldRotation(const DirectX::XMFLOAT4& world, bool recursive)
{
  auto parent = ParentWorldRotation();
  DirectX::XMStoreFloat4(
    &Transform.Rotation,
    DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&world),
                                  DirectX::XMQuaternionInverse(parent)));
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

std::shared_ptr<Node>
Node::GetShapeTail()
{
  auto parent = Parent.lock();
  if (!parent) {
    // root not use tail
    return nullptr;
  }

  switch (Children.size()) {
    case 0:
      return nullptr;

    case 1:
      return Children.front();
  }

  std::shared_ptr<Node> tail;
  for (auto& child : Children) {
    if (auto childHumanoid = child->Humanoid) {
      switch (childHumanoid->HumanBone) {
        case vrm::HumanBones::spine:
        case vrm::HumanBones::neck:
        case vrm::HumanBones::leftMiddleProximal:
        case vrm::HumanBones::rightMiddleProximal:
          return child;

        case vrm::HumanBones::leftEye:
        case vrm::HumanBones::rightEye:
          break;

        default:
          tail = child;
          break;
      }
    } else {
      if (!tail) {
        tail = child;
      } else if (std::abs(child->Transform.Translation.x) <
                 std::abs(tail->Transform.Translation.x)) {
        // coose center node
        tail = child;
      }
    }
  }
  return tail;
}

std::optional<vrm::HumanBones>
Node::ClosestBone()
{
  if (Humanoid) {
    return Humanoid->HumanBone;
  }
  if (AnyTail()) {
    for (auto current = Parent.lock(); current;
         current = current->Parent.lock()) {
      if (auto humanoid = current->Humanoid) {
        return humanoid->HumanBone;
      }
    }
  } else if (auto parent = Parent.lock()) {
    if (auto parentHumanoid = parent->Humanoid) {
      if (vrm::HumanBoneIsFinger(parentHumanoid->HumanBone)) {
        return parentHumanoid->HumanBone;
      }
    }
  }
  return {};
}

void
Node::CalcShape()
{
  float w = 0.02f;
  float d = 0.02f;
  // ShapeColor = { 1, 1, 1, 1 };
  auto humanBone = ClosestBone();
  if (humanBone) {
    ShapeColor = vrm::HumanBoneToColor(*humanBone);
    auto size = vrm::HumanBoneToWidthDepth(*humanBone);
    w = size.x;
    d = size.y;
  }

  DirectX::XMStoreFloat4x4(&ShapeMatrix, DirectX::XMMatrixScaling(w, w, w));

  if (Children.empty()) {
    return;
  }

  if (auto tail = GetShapeTail()) {
    auto _Y = DirectX::XMFLOAT3(tail->Transform.Translation.x,
                                tail->Transform.Translation.y,
                                tail->Transform.Translation.z);
    auto Y = DirectX::XMLoadFloat3(&_Y);

    auto length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Y));
    // std::cout << name_ << "=>" << tail->name_ << "=" << length <<
    // std::endl;
    Y = DirectX::XMVector3Normalize(Y);
    auto _Z = DirectX::XMFLOAT3(0, 0, 1);

    DirectX::XMVECTOR X;
    auto Z = DirectX::XMLoadFloat3(&_Z);
    auto dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(Y, Z));
    if (fabs(dot) < 0.9) {
      X = DirectX::XMVector3Cross(Y, Z);
      Z = DirectX::XMVector3Cross(X, Y);
    } else {
      auto _X = DirectX::XMFLOAT3(1, 0, 0);
      X = DirectX::XMLoadFloat3(&_X);
      Z = DirectX::XMVector3Cross(X, Y);
      X = DirectX::XMVector3Cross(Y, Z);
    }

    auto center = DirectX::XMMatrixTranslation(0, 0.5f, 0);
    auto scale = DirectX::XMMatrixScaling(w, length, d);
    DirectX::XMFLOAT4 _(0, 0, 0, 1);
    auto r = DirectX::XMMATRIX(X, Y, Z, DirectX::XMLoadFloat4(&_));

    auto shape = center * scale * r;
    DirectX::XMStoreFloat4x4(&ShapeMatrix, shape);
  }

  for (auto& child : Children) {
    child->CalcShape();
  }
}

void
Node::UpdateShapeInstanceRecursive(DirectX::XMMATRIX parent,
                                   const PushInstance& pushInstance)
{
  auto m = Transform.Matrix() * parent;
  auto shape = DirectX::XMLoadFloat4x4(&ShapeMatrix);

  Instance instance;
  DirectX::XMStoreFloat4x4(&instance.Matrix, shape * m);
  instance.Color = ShapeColor;
  pushInstance(instance);
  for (auto& child : Children) {
    child->UpdateShapeInstanceRecursive(m, pushInstance);
  }
}

} // namespace gltf
