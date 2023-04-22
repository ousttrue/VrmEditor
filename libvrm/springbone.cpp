#include "vrm/springbone.h"
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
  DirectX::XMStoreFloat3(
    &m_currentTailPosotion,
    DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&localTailPosition),
                                head->ParentWorldMatrix()));
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
      Head->ParentWorldRotation()))
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

inline DirectX::XMVECTOR
rotate_from_to(DirectX::XMFLOAT3 _lhs, DirectX::XMFLOAT3 _rhs)
{
  auto lhs = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&_lhs));
  auto rhs = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&_rhs));
  auto axis = DirectX::XMVector3Cross(lhs, rhs);
  auto dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(lhs, rhs));
  if (fabs(1 - dot) < 1e-4) {
    return { 0, 0, 0, 1 };
  }
  auto angle = acos(dot);
  return DirectX::XMQuaternionRotationAxis(axis, angle);
}

DirectX::XMVECTOR
SpringJoint::WorldPosToLocalRotation(const DirectX::XMVECTOR& nextTail) const
{
  DirectX::XMFLOAT3 localNextTail;

  DirectX::XMVECTOR det;
  DirectX::XMStoreFloat3(
    &localNextTail,
    DirectX::XMVector3Transform(
      nextTail,
      DirectX::XMMatrixInverse(
        &det,
        DirectX::XMMatrixMultiply(Head->InitialMatrix(),
                                  Head->ParentWorldMatrix()))));

  assert(DirectX::XMVectorGetX(det) != 0);

  auto r = rotate_from_to(m_initLocalTailDir, localNextTail);
  assert(!std::isnan(DirectX::XMVectorGetX(r)));
  return r;
}

void
SpringSolver::Add(const std::shared_ptr<gltf::Node>& head,
                  const DirectX::XMFLOAT3& tail,
                  float dragForce,
                  float stiffiness)
{
  Joints.push_back(SpringJoint(head, tail, dragForce, stiffiness));
}

void
SpringSolver::AddRecursive(const std::shared_ptr<gltf::Node>& node,
                           float dragForce,
                           float stiffiness)
{
  if (node->Children.size()) {
    for (auto& child : node->Children) {
      Add(node, child->Transform.Translation, dragForce, stiffiness);
      break;
    }
  } else {
    auto delta = node->WorldTransform.Translation - node->ParentWorldPosition();
    auto childPosition = DirectX::XMVectorAdd(
      DirectX::XMLoadFloat3(&node->WorldTransform.Translation),
      DirectX::XMVectorScale(
        DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&delta)), 0.07f));

    DirectX::XMFLOAT3 localTailPosition;
    DirectX::XMStoreFloat3(
      &localTailPosition,
      DirectX::XMVector3Transform(
        childPosition, DirectX::XMMatrixInverse(nullptr, node->WorldMatrix())));

    Add(node, localTailPosition, dragForce, stiffiness);
  }

  for (auto& child : node->Children) {
    AddRecursive(child, dragForce, stiffiness);
  }
}

void
SpringSolver::Update(Time delta)
{
  bool doUpdate = delta.count() > 0;
  if (!doUpdate) {
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

}
