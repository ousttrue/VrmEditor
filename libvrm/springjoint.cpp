#include "vrm/springjoint.h"
#include "vrm/springcollider.h"

namespace libvrm {
namespace vrm {

SpringJoint::SpringJoint(const std::shared_ptr<gltf::Node>& head,
                         const DirectX::XMFLOAT3& localTailPosition,
                         float dragForce,
                         float stiffiness,
                         float radius)
  : Head(head)
  , DragForce(dragForce)
  , Stiffiness(stiffiness)
  , Radius(radius)
{
  auto world =
    head->WorldTransformPoint(DirectX::XMLoadFloat3(&localTailPosition));
  DirectX::XMStoreFloat3(&m_currentTailPosotion, world);
  m_lastTailPosotion = m_currentTailPosotion;
  m_tailLength = DirectX::XMVectorGetX(
    DirectX::XMVector3Length(DirectX::XMLoadFloat3(&localTailPosition)));
  assert(m_tailLength);
  DirectX::XMStoreFloat3(
    &m_initLocalTailDir,
    DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&localTailPosition)));
}

void
SpringJoint::DrawGizmo(IGizmoDrawer* gizmo)
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
  gizmo->DrawSphere(m_currentTailPosotion, Radius, { 1, 0, 1, 1 });
  // gizmo->drawLine(currentTail, nextTail, {0, 1, 0, 1});

  gizmo->DrawLine(
    Head->WorldTransform.Translation, m_currentTailPosotion, { 1, 1, 0, 1 });

  if (Head->Children.size()) {
    gizmo->DrawSphere(Head->Children.front()->WorldTransform.Translation,
                      Radius,
                      { 1, 0, 0, 1 });
    gizmo->DrawLine(Head->WorldTransform.Translation,
                    Head->Children.front()->WorldTransform.Translation,
                    { 1, 0, 0, 1 });
  }
}

void
SpringJoint::Update(Time time, SpringCollision* collision)
{
  auto currentTail = DirectX::XMLoadFloat3(&m_currentTailPosotion);

  auto delta = DirectX::XMVectorSubtract(
    currentTail, DirectX::XMLoadFloat3(&m_lastTailPosotion));

  // verlet積分で次の位置を計算
  auto dragInv = std::max(0.0f, 1.0f - DragForce);
  auto nextTail = DirectX::XMVectorAdd(
    DirectX::XMVectorAdd(currentTail,
                         // 前フレームの移動を継続する
                         DirectX::XMVectorScale(delta, dragInv)),
    // 親の回転による子ボーンの移動目標
    DirectX::XMVector3Rotate(
      DirectX::XMVectorScale(DirectX::XMLoadFloat3(&m_initLocalTailDir),
                             static_cast<float>(Stiffiness * time.count())),
      DirectX::XMQuaternionMultiply(
        DirectX::XMLoadFloat4(&Head->InitialTransform.Rotation),
        Head->ParentWorldRotation())))
    // 外力による移動量
    // + external;
    ;
  assert(!std::isnan(DirectX::XMVectorGetX(nextTail)));

  nextTail = ConstraintTailPosition(nextTail);

  // collision
  while (auto position = collision->Collide(nextTail, Radius)) {
    nextTail = ConstraintTailPosition(*position);
  }

  // update
  DirectX::XMStoreFloat3(&m_currentTailPosotion, nextTail);
  DirectX::XMStoreFloat3(&m_lastTailPosotion, currentTail);

  auto position = DirectX::XMLoadFloat3(&Head->WorldTransform.Translation);
  auto nextTailDir =
    DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(nextTail, position));

  auto newRotation = WorldPosToLocalRotation(nextTailDir);
  assert(!std::isnan(DirectX::XMVectorGetX(newRotation)));
  // DirectX::XMStoreFloat4(&Head->Transform.Rotation, newLocalRotation);
  // Head->CalcWorldMatrix(false);
  Head->SetWorldRotation(newRotation);
  for (auto& child : Head->Children) {
    // ひとつ下まで
    child->CalcWorldMatrix(false);
  }
}

DirectX::XMVECTOR
SpringJoint::ConstraintTailPosition(const DirectX::XMVECTOR& tail)
{
  auto position = DirectX::XMLoadFloat3(&Head->WorldTransform.Translation);
  auto nextTailDir =
    DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(tail, position));
  auto nextTail = DirectX::XMVectorAdd(
    position, DirectX::XMVectorScale(nextTailDir, m_tailLength));
  assert(!std::isnan(DirectX::XMVectorGetX(nextTail)));
  return nextTail;
}

DirectX::XMVECTOR
SpringJoint::WorldPosToLocalRotation(const DirectX::XMVECTOR& nextTailDir) const
{
  auto rotation = DirectX::XMQuaternionMultiply(
    DirectX::XMLoadFloat4(&Head->InitialTransform.Rotation),
    Head->ParentWorldRotation());
  return DirectX::XMQuaternionMultiply(
    rotation,
    dmath::rotate_from_to(
      DirectX::XMVector3Rotate(DirectX::XMLoadFloat3(&m_initLocalTailDir),
                               rotation),
      nextTailDir));
}

}
}
