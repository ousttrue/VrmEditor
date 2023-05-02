#include "vrm/runtimescene/springjoint.h"
#include <vrm/dmath.h>

namespace runtimescene {

RuntimeSpringJoint::RuntimeSpringJoint(const libvrm::vrm::SpringJoint& joint)
{
  auto world = joint.Head->WorldInitialTransformPoint(
    DirectX::XMLoadFloat3(&joint.LocalTailPosition));
  DirectX::XMStoreFloat3(&m_currentTailPosotion, world);
  m_lastTailPosotion = m_currentTailPosotion;
  m_tailLength = DirectX::XMVectorGetX(
    DirectX::XMVector3Length(DirectX::XMLoadFloat3(&joint.LocalTailPosition)));
  assert(m_tailLength);
  DirectX::XMStoreFloat3(&m_initLocalTailDir,
                         DirectX::XMVector3Normalize(
                           DirectX::XMLoadFloat3(&joint.LocalTailPosition)));
}

void
RuntimeSpringJoint::DrawGizmo(const libvrm::vrm::SpringJoint& joint,
                              libvrm::IGizmoDrawer* gizmo)
{
  // gizmo->drawSphere(Head->worldPosition(), {1, 1, 1, 1});
  // gizmo->drawSphere(m_currentTailPosotion, {0, 1, 0, 1});
  // gizmo->drawLine(Head->worldPosition(), m_currentTailPosotion, {0, 1, 0,
  // 1});
  //
  // gizmo->drawSphere(m_lastTailPosotion, {1, 0, 0, 1});
  // gizmo->drawLine(Head->worldPosition(), m_lastTailPosotion, {1, 0, 0, 1});

  // gizmo->drawSphere(lastHead, {1, 1, 1, 1});
  // gizmo->DrawSphere(Head->WorldTransform.Translation, Radius, { 0, 1, 0, 1
  // }); gizmo->drawLine(lastHead, Head->worldPosition(), {1, 0, 1, 1});

  // gizmo->drawSphere(currentTail, {1, 1, 1, 1});
  gizmo->DrawSphere(m_currentTailPosotion, joint.Radius, { 1, 0, 1, 1 });
  // gizmo->drawLine(currentTail, nextTail, {0, 1, 0, 1});

  gizmo->DrawLine(joint.Head->WorldTransform.Translation,
                  m_currentTailPosotion,
                  { 1, 1, 0, 1 });

  if (joint.Head->Children.size()) {
    gizmo->DrawSphere(joint.Head->Children.front()->WorldTransform.Translation,
                      joint.Radius,
                      { 1, 0, 0, 1 });
    gizmo->DrawLine(joint.Head->WorldTransform.Translation,
                    joint.Head->Children.front()->WorldTransform.Translation,
                    { 1, 0, 0, 1 });
  }
}

void
RuntimeSpringJoint::Update(const libvrm::vrm::SpringJoint& joint,
                           libvrm::Time time,
                           libvrm::vrm::SpringCollision* collision)
{
  auto currentTail = DirectX::XMLoadFloat3(&m_currentTailPosotion);

  auto delta = DirectX::XMVectorSubtract(
    currentTail, DirectX::XMLoadFloat3(&m_lastTailPosotion));

  // verlet積分で次の位置を計算
  auto dragInv = std::max(0.0f, 1.0f - joint.DragForce);
  auto nextTail = DirectX::XMVectorAdd(
    DirectX::XMVectorAdd(currentTail,
                         // 前フレームの移動を継続する
                         DirectX::XMVectorScale(delta, dragInv)),
    // 親の回転による子ボーンの移動目標
    DirectX::XMVector3Rotate(
      DirectX::XMVectorScale(
        DirectX::XMLoadFloat3(&m_initLocalTailDir),
        static_cast<float>(joint.Stiffness * time.count())),
      DirectX::XMQuaternionMultiply(
        DirectX::XMLoadFloat4(&joint.Head->InitialTransform.Rotation),
        joint.Head->ParentWorldRotation())))
    // 外力による移動量
    // + external;
    ;
  assert(!std::isnan(DirectX::XMVectorGetX(nextTail)));

  nextTail = ConstraintTailPosition(joint, nextTail);

  // collision
  while (auto position = collision->Collide(nextTail, joint.Radius)) {
    nextTail = ConstraintTailPosition(joint, *position);
  }

  // update
  DirectX::XMStoreFloat3(&m_currentTailPosotion, nextTail);
  DirectX::XMStoreFloat3(&m_lastTailPosotion, currentTail);

  auto position =
    DirectX::XMLoadFloat3(&joint.Head->WorldTransform.Translation);
  auto nextTailDir =
    DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(nextTail, position));

  auto newRotation = WorldPosToLocalRotation(joint, nextTailDir);
  assert(!std::isnan(DirectX::XMVectorGetX(newRotation)));
  // DirectX::XMStoreFloat4(&Head->Transform.Rotation, newLocalRotation);
  // Head->CalcWorldMatrix(false);
  joint.Head->SetWorldRotation(newRotation);
  for (auto& child : joint.Head->Children) {
    // ひとつ下まで
    child->CalcWorldMatrix(false);
  }
}

DirectX::XMVECTOR
RuntimeSpringJoint::ConstraintTailPosition(
  const libvrm::vrm::SpringJoint& joint,
  const DirectX::XMVECTOR& tail)
{
  auto position =
    DirectX::XMLoadFloat3(&joint.Head->WorldTransform.Translation);
  auto nextTailDir =
    DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(tail, position));
  auto nextTail = DirectX::XMVectorAdd(
    position, DirectX::XMVectorScale(nextTailDir, m_tailLength));
  assert(!std::isnan(DirectX::XMVectorGetX(nextTail)));
  return nextTail;
}

DirectX::XMVECTOR
RuntimeSpringJoint::WorldPosToLocalRotation(
  const libvrm::vrm::SpringJoint& joint,
  const DirectX::XMVECTOR& nextTailDir) const
{
  auto rotation = DirectX::XMQuaternionMultiply(
    DirectX::XMLoadFloat4(&joint.Head->InitialTransform.Rotation),
    joint.Head->ParentWorldRotation());
  return DirectX::XMQuaternionMultiply(
    rotation,
    dmath::rotate_from_to(
      DirectX::XMVector3Rotate(DirectX::XMLoadFloat3(&m_initLocalTailDir),
                               rotation),
      nextTailDir));
}

}
