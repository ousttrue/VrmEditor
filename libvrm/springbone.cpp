#include "vrm/springbone.h"
#include "vrm/dmath.h"
#include "vrm/gizmo.h"
#include "vrm/scenetypes.h"
#include <iostream>

namespace libvrm::vrm {

SpringJoint::SpringJoint(const std::shared_ptr<gltf::Node>& head,
                         const DirectX::XMFLOAT3& localTailPosition,
                         float dragForce,
                         float stiffiness)
  : Head(head)
  , DragForce(dragForce)
  , Stiffiness(stiffiness)
{

  m_currentTailPosotion =
    dmath::transform(localTailPosition, head->ParentWorldMatrix());
  m_lastTailPosotion = m_currentTailPosotion;
  m_tailLength = dmath::length(localTailPosition);
  assert(m_tailLength);
  m_initLocalTailDir = dmath::normalized(localTailPosition);
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
  auto currentTail = m_currentTailPosotion;

  auto delta = currentTail - m_lastTailPosotion;

  // verlet積分で次の位置を計算
  auto nextTail = currentTail
                  // 前フレームの移動を継続する
                  + delta * (1.0f - DragForce)
                  // 親の回転による子ボーンの移動目標
                  +
                  dmath::rotate(m_initLocalTailDir * Stiffiness * time.count(),
                                Head->ParentWorldRotation())
    // 外力による移動量
    // + external;
    ;

  assert(!std::isnan(nextTail.x));

  auto position = Head->WorldTransform.Translation;
  nextTail = position + dmath::normalized(nextTail - position) * m_tailLength;

  // auto head = Head.get();
  assert(!std::isnan(nextTail.x));

  // update
  m_currentTailPosotion = nextTail;
  m_lastTailPosotion = currentTail;
  auto newLocalRotation = WorldPosToLocalRotation(nextTail);
  assert(!std::isnan(newLocalRotation.x));
  Head->Transform.Rotation = newLocalRotation;
  Head->CalcWorldMatrix(false);
  for (auto& child : Head->Children) {
    // ひとつ下まで
    child->CalcWorldMatrix(false);
  }
}

DirectX::XMFLOAT4
SpringJoint::WorldPosToLocalRotation(const DirectX::XMFLOAT3& nextTail) const
{
  DirectX::XMFLOAT3 localNextTail;

  DirectX::XMVECTOR det;
  DirectX::XMStoreFloat3(
    &localNextTail,
    DirectX::XMVector3Transform(
      DirectX::XMLoadFloat3(&nextTail),
      DirectX::XMMatrixInverse(
        &det,
        DirectX::XMMatrixMultiply(Head->InitialMatrix(),
                                  Head->ParentWorldMatrix()))));

  assert(DirectX::XMVectorGetX(det) != 0);

  auto r = dmath::rotate_from_to(m_initLocalTailDir, localNextTail);
  assert(!std::isnan(r.x));
  return r;
}

void
SpringSolver::AddRecursive(const std::shared_ptr<gltf::Node>& node,
                           float dragForce,
                           float stiffiness)
{

  DirectX::XMFLOAT3 localTailPosition;
  if (node->Children.size()) {
    for (auto& child : node->Children) {
      localTailPosition = child->Transform.Translation;
      Joints.push_back(
        SpringJoint(node, localTailPosition, dragForce, stiffiness));
      break;
    }
  } else {
    auto delta = node->WorldTransform.Translation - node->ParentWorldPosition();
    auto childPosition =
      node->WorldTransform.Translation + dmath::normalized(delta) * 0.07f;

    DirectX::XMStoreFloat3(
      &localTailPosition,
      DirectX::XMVector3Transform(
        DirectX::XMLoadFloat3(&childPosition),
        DirectX::XMMatrixInverse(nullptr, node->WorldMatrix())));
  }

  for (auto& child : node->Children) {
    AddRecursive(child, dragForce, stiffiness);
  }
}

void
SpringSolver::Update(Time time)
{
  bool doUpdate = time != LastTime;
  auto delta = time - LastTime;
  LastTime = time;
  if (!doUpdate) {
    return;
  }
  if (delta.count() == 0) {
    return;
  }

  for (auto& joint : Joints) {
    joint.Update(delta);
  }
}

void
SpringSolver::DrawGizmo(IGizmoDrawer* gizmo)
{
  for (auto& joint : Joints) {
    joint.DrawGizmo(gizmo);
  }
}

} // namespace vrm
