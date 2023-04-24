#include "vrm/springjoint.h"

namespace libvrm {
namespace vrm {

SpringJoint::SpringJoint(const std::shared_ptr<gltf::Node>& head,
                         const DirectX::XMFLOAT3& localTailPosition,
                         float dragForce,
                         float stiffiness)
  : Head(head)
  , DragForce(dragForce)
  , Stiffiness(stiffiness)
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
  gizmo->DrawSphere(Head->WorldTransform.Translation, { 0, 1, 0, 1 });
  // gizmo->drawLine(lastHead, Head->worldPosition(), {1, 0, 1, 1});

  // gizmo->drawSphere(currentTail, {1, 1, 1, 1});
  gizmo->DrawSphere(m_currentTailPosotion, { 1, 0, 1, 1 });
  // gizmo->drawLine(currentTail, nextTail, {0, 1, 0, 1});

  gizmo->DrawLine(
    Head->WorldTransform.Translation, m_currentTailPosotion, { 1, 1, 0, 1 });

  if (Head->Children.size()) {
    gizmo->DrawSphere(Head->Children.front()->WorldTransform.Translation,
                      { 1, 0, 0, 1 });
    gizmo->DrawLine(Head->WorldTransform.Translation,
                    Head->Children.front()->WorldTransform.Translation,
                    { 1, 0, 0, 1 });
  }
}

void
SpringJoint::Update(Time time)
{
  auto currentTail = DirectX::XMLoadFloat3(&m_currentTailPosotion);

  auto delta = DirectX::XMVectorSubtract(
    currentTail, DirectX::XMLoadFloat3(&m_lastTailPosotion));

  // verlet積分で次の位置を計算
  auto nextTail = DirectX::XMVectorAdd(
    DirectX::XMVectorAdd(currentTail,
                         // 前フレームの移動を継続する
                         DirectX::XMVectorScale(delta, 1.0f - DragForce))
    // 親の回転による子ボーンの移動目標
    ,
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

  auto position = DirectX::XMLoadFloat3(&Head->WorldTransform.Translation);
  nextTail = DirectX::XMVectorAdd(
    position,
    DirectX::XMVectorScale(DirectX::XMVector3Normalize(
                             DirectX::XMVectorSubtract(nextTail, position)),
                           m_tailLength));

  // auto head = Head.get();
  assert(!std::isnan(DirectX::XMVectorGetX(nextTail)));

  // update
  DirectX::XMStoreFloat3(&m_currentTailPosotion, nextTail);
  DirectX::XMStoreFloat3(&m_lastTailPosotion, currentTail);
  auto newLocalRotation = WorldPosToLocalRotation(nextTail);
  assert(!std::isnan(DirectX::XMVectorGetX(newLocalRotation)));
  DirectX::XMStoreFloat4(&Head->Transform.Rotation, newLocalRotation);
  Head->CalcWorldMatrix(false);
  for (auto& child : Head->Children) {
    // ひとつ下まで
    child->CalcWorldMatrix(false);
  }
}

DirectX::XMVECTOR
SpringJoint::WorldPosToLocalRotation(const DirectX::XMVECTOR& nextTail) const
{
  DirectX::XMVECTOR det;
  auto m = DirectX::XMMatrixInverse(
    &det,
    DirectX::XMMatrixTranslationFromVector(
      DirectX::XMLoadFloat3(&Head->InitialTransform.Translation)) *
      Head->ParentWorldMatrix());

  auto localNextTail = DirectX::XMVector3Transform(nextTail, m);

  assert(DirectX::XMVectorGetX(det) != 0);

  auto r = dmath::rotate_from_to(DirectX::XMLoadFloat3(&m_initLocalTailDir),
                                 localNextTail);
  assert(!std::isnan(DirectX::XMVectorGetX(r)));
  return r;
}

}
}
