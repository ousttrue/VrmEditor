#include "runtime_springJoint.h"
#include "runtime_node.h"
#include "runtime_scene.h"
#include "spring_collision.h"
#include <vrm/dmath.h>

namespace runtimescene {

RuntimeSpringJoint::RuntimeSpringJoint(
  const std::shared_ptr<libvrm::SpringJoint>& joint)
  : Joint(joint)
{
  auto world = joint->Head->WorldInitialTransformPoint(
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

void
RuntimeSpringJoint::DrawGizmo(RuntimeScene* runtime,
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
  gizmo->DrawSphere(m_currentTailPosotion, Joint->Radius, { 1, 0, 1, 1 });
  // gizmo->drawLine(currentTail, nextTail, {0, 1, 0, 1});

  gizmo->DrawLine(
    runtime->GetRuntimeNode(Joint->Head)->WorldTransform.Translation,
    m_currentTailPosotion,
    { 1, 1, 0, 1 });

  if (Joint->Head->Children.size()) {
    gizmo->DrawSphere(runtime->GetRuntimeNode(Joint->Head->Children.front())
                        ->WorldTransform.Translation,
                      Joint->Radius,
                      { 1, 0, 0, 1 });
    gizmo->DrawLine(
      runtime->GetRuntimeNode(Joint->Head)->WorldTransform.Translation,
      runtime->GetRuntimeNode(Joint->Head->Children.front())
        ->WorldTransform.Translation,
      { 1, 0, 0, 1 });
  }
}

void
RuntimeSpringJoint::Update(RuntimeScene* runtime,
                           libvrm::Time time,
                           RuntimeSpringCollision* collision)
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
        DirectX::XMLoadFloat4(&Joint->Head->InitialTransform.Rotation),
        runtime->GetRuntimeNode(Joint->Head)->ParentWorldRotation())))
    // 外力による移動量
    // + external;
    ;
  assert(!std::isnan(DirectX::XMVectorGetX(nextTail)));

  nextTail = ConstraintTailPosition(runtime, nextTail);

  // collision
  while (auto position = collision->Collide(nextTail, Joint->Radius)) {
    nextTail = ConstraintTailPosition(runtime, *position);
  }

  // update
  DirectX::XMStoreFloat3(&m_currentTailPosotion, nextTail);
  DirectX::XMStoreFloat3(&m_lastTailPosotion, currentTail);

  auto position = DirectX::XMLoadFloat3(
    &runtime->GetRuntimeNode(Joint->Head)->WorldTransform.Translation);
  auto nextTailDir =
    DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(nextTail, position));

  auto newRotation = WorldPosToLocalRotation(runtime, nextTailDir);
  assert(!std::isnan(DirectX::XMVectorGetX(newRotation)));
  // DirectX::XMStoreFloat4(&Head->Transform.Rotation, newLocalRotation);
  // Head->CalcWorldMatrix(false);
  runtime->GetRuntimeNode(Joint->Head)->SetWorldRotation(newRotation);
  for (auto& child : Joint->Head->Children) {
    // ひとつ下まで
    runtime->GetRuntimeNode(child)->CalcWorldMatrix(false);
  }
}

DirectX::XMVECTOR
RuntimeSpringJoint::ConstraintTailPosition(RuntimeScene* runtime,
                                           const DirectX::XMVECTOR& tail)
{
  auto position = DirectX::XMLoadFloat3(
    &runtime->GetRuntimeNode(Joint->Head)->WorldTransform.Translation);
  auto nextTailDir =
    DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(tail, position));
  auto nextTail = DirectX::XMVectorAdd(
    position, DirectX::XMVectorScale(nextTailDir, m_tailLength));
  assert(!std::isnan(DirectX::XMVectorGetX(nextTail)));
  return nextTail;
}

DirectX::XMVECTOR
RuntimeSpringJoint::WorldPosToLocalRotation(
  RuntimeScene* runtime,
  const DirectX::XMVECTOR& nextTailDir) const
{
  auto rotation = DirectX::XMQuaternionMultiply(
    DirectX::XMLoadFloat4(&Joint->Head->InitialTransform.Rotation),
    runtime->GetRuntimeNode(Joint->Head)->ParentWorldRotation());
  return DirectX::XMQuaternionMultiply(
    rotation,
    dmath::rotate_from_to(
      DirectX::XMVector3Rotate(DirectX::XMLoadFloat3(&m_initLocalTailDir),
                               rotation),
      nextTailDir));
}

}
