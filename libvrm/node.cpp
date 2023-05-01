#include "vrm/node.h"
#include "vrm/dmath.h"
#include <iostream>

namespace libvrm {
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
Node::SetWorldRotation(const DirectX::XMVECTOR& world, bool recursive)
{
  auto parent = ParentWorldRotation();
  DirectX::XMStoreFloat4(
    &Transform.Rotation,
    DirectX::XMQuaternionMultiply(world, DirectX::XMQuaternionInverse(parent)));
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
  if (Children.empty()) {
    return nullptr;
  }

  std::shared_ptr<Node> tail;
  for (auto& child : Children) {
    if (auto childHumanBone = child->Humanoid) {
      switch (*childHumanBone) {
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

  for (auto current = Parent.lock(); current;
       current = current->Parent.lock()) {
    if (current->Humanoid) {
      return tail;
    }
  }
  return nullptr;
}

std::optional<vrm::HumanBones>
Node::ClosestBone()
{
  if (Humanoid) {
    return *Humanoid;
  }
  if (AnyTail()) {
    for (auto current = Parent.lock(); current;
         current = current->Parent.lock()) {
      if (auto humanoid = current->Humanoid) {
        return *humanoid;
      }
    }
  } else if (auto parent = Parent.lock()) {
    if (auto parentHumanoid = parent->Humanoid) {
      if (vrm::HumanBoneIsFinger(*parentHumanoid)) {
        return *parentHumanoid;
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
  if (Humanoid) {
    ShapeColor = vrm::HumanBoneToColor(*Humanoid);
    auto size = vrm::HumanBoneToWidthDepth(*Humanoid);
    w = size.x;
    d = size.y;
  } else if (auto humanBone = ClosestBone()) {
    auto size = vrm::HumanBoneToWidthDepth(*humanBone);
    w = size.x;
    d = size.y;
  }

  if (Children.empty()) {
    // ShapeColor = { 0.2f, 0.2f, 0.2f, 1 };
    DirectX::XMStoreFloat4x4(&ShapeMatrix, DirectX::XMMatrixScaling(w, w, w));
    return;
  }

  if (auto tail = GetShapeTail()) {
    auto _Y = DirectX::XMFLOAT3(tail->Transform.Translation.x,
                                tail->Transform.Translation.y,
                                tail->Transform.Translation.z);
    auto Y = DirectX::XMLoadFloat3(&_Y);
    auto length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Y));
    Y = DirectX::XMVector3Normalize(Y);

    auto _Z = DirectX::XMFLOAT3(0, 0, 1);
    auto Z = DirectX::XMLoadFloat3(&_Z);

    auto dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(Y, Z));
    DirectX::XMVECTOR X;
    if (fabs(dot) < 0.9) {
      X = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(Y, Z));
      Z = DirectX::XMVector3Cross(X, Y);
    } else {
      auto _X = DirectX::XMFLOAT3(1, 0, 0);
      X = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&_X));
      Z = DirectX::XMVector3Cross(X, Y);
      X = DirectX::XMVector3Cross(Y, Z);
    }

    auto center = DirectX::XMMatrixTranslation(0, 0.5f, 0);
    auto scale = DirectX::XMMatrixScaling(w, length, d);
    DirectX::XMFLOAT4 _(0, 0, 0, 1);
    auto r = DirectX::XMMATRIX(X, Y, Z, DirectX::XMLoadFloat4(&_));

    auto shape = center * scale * r;
    DirectX::XMStoreFloat4x4(&ShapeMatrix, shape);
  } else {
    ShapeColor = { 1, 0.2f, 0.2f, 1 };
    DirectX::XMStoreFloat4x4(&ShapeMatrix, DirectX::XMMatrixScaling(w, w, w));
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

static void
Constraint_Rotation(const std::shared_ptr<Node>& src,
                    const std::shared_ptr<Node>& dst,
                    float weight)
{
  auto delta = DirectX::XMQuaternionMultiply(
    DirectX::XMLoadFloat4(&src->Transform.Rotation),
    DirectX::XMQuaternionInverse(
      DirectX::XMLoadFloat4(&src->InitialTransform.Rotation)));

  DirectX::XMStoreFloat4(
    &dst->Transform.Rotation,
    DirectX::XMQuaternionSlerp(
      DirectX::XMLoadFloat4(&dst->InitialTransform.Rotation),
      DirectX::XMQuaternionMultiply(
        delta, DirectX::XMLoadFloat4(&dst->InitialTransform.Rotation)),
      weight));
}

static DirectX::XMVECTOR
mul3(DirectX::XMVECTOR q0, DirectX::XMVECTOR q1, DirectX::XMVECTOR q2)
{
  return DirectX::XMQuaternionMultiply(DirectX::XMQuaternionMultiply(q0, q1),
                                       q2);
}

static DirectX::XMVECTOR
mul4(DirectX::XMVECTOR q0,
     DirectX::XMVECTOR q1,
     DirectX::XMVECTOR q2,
     DirectX::XMVECTOR q3)
{
  return DirectX::XMQuaternionMultiply(mul3(q0, q1, q2), q3);
}

static void
Constraint_Aim(const std::shared_ptr<Node>& src,
               const std::shared_ptr<Node>& dst,
               float weight,
               const DirectX::XMVECTOR axis)
{
  auto dstParentWorldQuat = dst->ParentWorldRotation();
  auto fromVec = DirectX::XMVector3Rotate(
    axis,
    DirectX::XMQuaternionMultiply(
      DirectX::XMLoadFloat4(&dst->InitialTransform.Rotation),
      dstParentWorldQuat));
  auto toVec = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(
    DirectX::XMLoadFloat3(&src->WorldTransform.Translation),
    DirectX::XMLoadFloat3(&dst->WorldTransform.Translation)));
  auto fromToQuat = dmath::rotate_from_to(fromVec, toVec);

  DirectX::XMStoreFloat4(
    &dst->Transform.Rotation,
    DirectX::XMQuaternionSlerp(
      DirectX::XMLoadFloat4(&dst->InitialTransform.Rotation),
      mul4(DirectX::XMLoadFloat4(&dst->InitialTransform.Rotation),
           dstParentWorldQuat,
           fromToQuat,
           DirectX::XMQuaternionInverse(dstParentWorldQuat)),
      weight));
}

static void
Constraint_Roll(const std::shared_ptr<Node>& src,
                const std::shared_ptr<Node>& dst,
                float weight,
                DirectX::XMVECTOR axis)
{
  auto deltaSrcQuat = DirectX::XMQuaternionMultiply(
    DirectX::XMLoadFloat4(&src->Transform.Rotation),
    DirectX::XMQuaternionInverse(
      DirectX::XMLoadFloat4(&src->InitialTransform.Rotation)));
  auto deltaSrcQuatInParent =
    mul3(DirectX::XMQuaternionInverse(
           DirectX::XMLoadFloat4(&src->InitialTransform.Rotation)),
         deltaSrcQuat,
         DirectX::XMLoadFloat4(&src->InitialTransform.Rotation));
  auto deltaSrcQuatInDst =
    mul3(DirectX::XMLoadFloat4(&dst->InitialTransform.Rotation),
         deltaSrcQuatInParent,
         DirectX::XMQuaternionInverse(
           DirectX::XMLoadFloat4(&dst->InitialTransform.Rotation)));
  auto toVec = DirectX::XMQuaternionMultiply(axis, deltaSrcQuatInDst);
  auto fromToQuat = dmath::rotate_from_to(axis, toVec);

  DirectX::XMStoreFloat4(
    &dst->Transform.Rotation,
    DirectX::XMQuaternionSlerp(
      DirectX::XMLoadFloat4(&dst->InitialTransform.Rotation),
      DirectX::XMQuaternionMultiply(DirectX::XMQuaternionInverse(fromToQuat),
                                    deltaSrcQuatInDst),
      weight));
}

void
NodeConstraint::Process(const std::shared_ptr<Node>& dst)
{
  auto src = Source.lock();
  if (!src) {
    return;
  }

  switch (Type) {
    case NodeConstraintTypes::Rotation:
      Constraint_Rotation(src, dst, Weight);
      break;

    case NodeConstraintTypes::Roll:
      Constraint_Roll(src, dst, Weight, GetRollVector(RollAxis));
      break;

    case NodeConstraintTypes::Aim:
      Constraint_Aim(src, dst, Weight, GetAxisVector(AimAxis));
      break;
  }
}

}
}
