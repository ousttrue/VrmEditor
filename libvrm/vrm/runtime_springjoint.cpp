#include "runtime_springJoint.h"
#include "gizmo.h"
#include "runtime_node.h"
#include "spring_collision.h"
#include <vrm/dmath.h>

namespace libvrm {

RuntimeSpringJoint::RuntimeSpringJoint(
  const std::shared_ptr<libvrm::SpringJoint>& joint)
  : Joint(joint)
{
  auto world = joint->Head->Base->WorldInitialTransformPoint(
    DirectX::XMLoadFloat3(&joint->LocalTailPosition));
  DirectX::XMStoreFloat3(&m_currentTailPosotion, world);
  m_lastTailPosotion = m_currentTailPosotion;
  m_tailLength = DirectX::XMVectorGetX(
    DirectX::XMVector3Length(DirectX::XMLoadFloat3(&joint->LocalTailPosition)));
  assert(m_tailLength);
  DirectX::XMStoreFloat3(&m_initLocalTailDir,
                         DirectX::XMVector3Normalize(
                           DirectX::XMLoadFloat3(&joint->LocalTailPosition)));
}

const DirectX::XMFLOAT4 CYAN = { 1, 1, 0, 1 };
const DirectX::XMFLOAT4 RED = { 1, 0, 0, 1 };

void
RuntimeSpringJoint::DrawGizmo(libvrm::IGizmoDrawer* gizmo, const DirectX::XMFLOAT4 &color)
{
  gizmo->DrawSphere(
    m_currentTailPosotion, Joint->Radius, color);
  gizmo->DrawLine(
    Joint->Head->WorldTransform.Translation, m_currentTailPosotion, CYAN);

  // if (Joint->Head->Children.size()) {
  //   gizmo->DrawSphere(Joint->Head->Children.front()->WorldTransform.Translation,
  //                     Joint->Radius,
  //                     RED);
  //   gizmo->DrawLine(Joint->Head->WorldTransform.Translation,
  //                   Joint->Head->Children.front()->WorldTransform.Translation,
  //                   RED);
  // }
}

void
RuntimeSpringJoint::Update(libvrm::Time time, RuntimeSpringCollision* collision)
{
  auto currentTail = DirectX::XMLoadFloat3(&m_currentTailPosotion);

  auto delta = DirectX::XMVectorSubtract(
    currentTail, DirectX::XMLoadFloat3(&m_lastTailPosotion));

  // verlet積分で次の位置を計算
  auto dragInv = std::max(0.0f, 1.0f - Joint->DragForce);
  auto nextTail = DirectX::XMVectorAdd(
    DirectX::XMVectorAdd(currentTail,
                         // 前フレームの移動を継続する
                         DirectX::XMVectorScale(delta, dragInv)),
    // 親の回転による子ボーンの移動目標
    DirectX::XMVector3Rotate(
      DirectX::XMVectorScale(
        DirectX::XMLoadFloat3(&m_initLocalTailDir),
        static_cast<float>(Joint->Stiffness * time.count())),
      DirectX::XMQuaternionMultiply(
        DirectX::XMLoadFloat4(&Joint->Head->Base->InitialTransform.Rotation),
        Joint->Head->ParentWorldRotation())))
    // 外力による移動量
    // + external;
    ;
  assert(!std::isnan(DirectX::XMVectorGetX(nextTail)));

  nextTail = ConstraintTailPosition(nextTail);

  // collision
  while (auto position = collision->Collide(nextTail, Joint->Radius)) {
    nextTail = ConstraintTailPosition(*position);
  }

  // update
  DirectX::XMStoreFloat3(&m_currentTailPosotion, nextTail);
  DirectX::XMStoreFloat3(&m_lastTailPosotion, currentTail);

  auto position =
    DirectX::XMLoadFloat3(&Joint->Head->WorldTransform.Translation);
  auto nextTailDir =
    DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(nextTail, position));

  auto newRotation = WorldPosToLocalRotation(nextTailDir);
  assert(!std::isnan(DirectX::XMVectorGetX(newRotation)));
  // DirectX::XMStoreFloat4(&Head->Transform.Rotation, newLocalRotation);
  // Head->CalcWorldMatrix(false);
  Joint->Head->SetWorldRotation(newRotation);
  for (auto& child : Joint->Head->Children) {
    // ひとつ下まで
    child->CalcWorldMatrix(false);
  }
}

DirectX::XMVECTOR
RuntimeSpringJoint::ConstraintTailPosition(const DirectX::XMVECTOR& tail)
{
  auto position =
    DirectX::XMLoadFloat3(&Joint->Head->WorldTransform.Translation);
  auto nextTailDir =
    DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(tail, position));
  auto nextTail = DirectX::XMVectorAdd(
    position, DirectX::XMVectorScale(nextTailDir, m_tailLength));
  assert(!std::isnan(DirectX::XMVectorGetX(nextTail)));
  return nextTail;
}

DirectX::XMVECTOR
RuntimeSpringJoint::WorldPosToLocalRotation(
  const DirectX::XMVECTOR& nextTailDir) const
{
  auto rotation = DirectX::XMQuaternionMultiply(
    DirectX::XMLoadFloat4(&Joint->Head->Base->InitialTransform.Rotation),
    Joint->Head->ParentWorldRotation());
  return DirectX::XMQuaternionMultiply(
    rotation,
    dmath::rotate_from_to(
      DirectX::XMVector3Rotate(DirectX::XMLoadFloat3(&m_initLocalTailDir),
                               rotation),
      nextTailDir));
}

}
