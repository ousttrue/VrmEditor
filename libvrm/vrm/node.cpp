#include "vrm/node.h"
#include "vrm/dmath.h"
#include <iostream>

namespace libvrm {

Node::Node(std::string_view name)
  : Name(name)
{
}

// void
// Node::CalcInitialMatrix()
// {
//   WorldInitialTransform = WorldTransform;
//   WorldInitialScale = WorldScale;
//   InitialTransform = Transform;
//   InitialScale = Scale;
// }

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

// void
// Node::Print(int level)
// {
//   for (int i = 0; i < level; ++i) {
//     std::cout << "  ";
//   }
//   std::cout << Name << std::endl;
//   for (auto child : Children) {
//     child->Print(level + 1);
//   }
// }

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
        case HumanBones::spine:
        case HumanBones::neck:
        case HumanBones::leftMiddleProximal:
        case HumanBones::rightMiddleProximal:
          return child;

        case HumanBones::leftEye:
        case HumanBones::rightEye:
          break;

        default:
          tail = child;
          break;
      }
    } else {
      if (!tail) {
        tail = child;
      } else if (std::abs(child->InitialTransform.Translation.x) <
                 std::abs(tail->InitialTransform.Translation.x)) {
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

std::optional<HumanBones>
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
      if (HumanBoneIsFinger(*parentHumanoid)) {
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
    ShapeColor = HumanBoneToColor(*Humanoid);
    auto size = HumanBoneToWidthDepth(*Humanoid);
    w = size.x;
    d = size.y;
  } else if (auto humanBone = ClosestBone()) {
    auto size = HumanBoneToWidthDepth(*humanBone);
    w = size.x;
    d = size.y;
  }

  if (Children.empty()) {
    // ShapeColor = { 0.2f, 0.2f, 0.2f, 1 };
    DirectX::XMStoreFloat4x4(&ShapeMatrix, DirectX::XMMatrixScaling(w, w, w));
    return;
  }

  if (auto tail = GetShapeTail()) {
    auto _Y = DirectX::XMFLOAT3(tail->InitialTransform.Translation.x,
                                tail->InitialTransform.Translation.y,
                                tail->InitialTransform.Translation.z);
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

// static void
// Constraint_Rotation(const std::shared_ptr<Node>& src,
//                     const std::shared_ptr<Node>& dst,
//                     float weight)
// {
//   auto delta = DirectX::XMQuaternionMultiply(
//     DirectX::XMLoadFloat4(&src->Transform.Rotation),
//     DirectX::XMQuaternionInverse(
//       DirectX::XMLoadFloat4(&src->InitialTransform.Rotation)));
//
//   DirectX::XMStoreFloat4(
//     &dst->Transform.Rotation,
//     DirectX::XMQuaternionSlerp(
//       DirectX::XMLoadFloat4(&dst->InitialTransform.Rotation),
//       DirectX::XMQuaternionMultiply(
//         delta, DirectX::XMLoadFloat4(&dst->InitialTransform.Rotation)),
//       weight));
// }

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

} // namespace
