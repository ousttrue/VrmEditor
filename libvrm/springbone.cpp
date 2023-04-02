#include "vrm/springbone.h"
#include "vrm/dmath.h"
#include "vrm/gizmo.h"
#include "vrm/scenetypes.h"
#include <iostream>

namespace vrm {

SpringJoint::SpringJoint(const std::shared_ptr<gltf::Node>& head,
                         const DirectX::XMFLOAT3& localTailPosition,
                         float dragForce,
                         float stiffiness)
  : Head(head)
  , DragForce(dragForce)
  , Stiffiness(stiffiness)
{

  m_currentTailPosotion =
    dmath::transform(localTailPosition, head->parentWorld());
  m_lastTailPosotion = m_currentTailPosotion;
  m_tailLength = dmath::length(localTailPosition);
  assert(m_tailLength);
  m_initLocalTailDir = dmath::normalized(localTailPosition);
}

void
SpringJoint::DrawGizmo()
{
  // gizmo::drawSphere(Head->worldPosition(), {1, 1, 1, 1});
  // gizmo::drawSphere(m_currentTailPosotion, {0, 1, 0, 1});
  // gizmo::drawLine(Head->worldPosition(), m_currentTailPosotion, {0, 1, 0,
  // 1});
  //
  // gizmo::drawSphere(m_lastTailPosotion, {1, 0, 0, 1});
  // gizmo::drawLine(Head->worldPosition(), m_lastTailPosotion, {1, 0, 0, 1});

  // gizmo::drawSphere(lastHead, {1, 1, 1, 1});
  gizmo::drawSphere(Head->worldPosition(), { 0, 1, 0, 1 });
  // gizmo::drawLine(lastHead, Head->worldPosition(), {1, 0, 1, 1});

  // gizmo::drawSphere(currentTail, {1, 1, 1, 1});
  gizmo::drawSphere(m_currentTailPosotion, { 1, 0, 1, 1 });
  // gizmo::drawLine(currentTail, nextTail, {0, 1, 0, 1});

  gizmo::drawLine(Head->worldPosition(), m_currentTailPosotion, { 1, 1, 0, 1 });

  if (Head->children.size()) {
    gizmo::drawSphere(Head->children.front()->worldPosition(), { 1, 0, 0, 1 });
    gizmo::drawLine(Head->worldPosition(),
                    Head->children.front()->worldPosition(),
                    { 1, 0, 0, 1 });
  }
}

void
SpringJoint::Update()
{
  auto currentTail = m_currentTailPosotion;

  auto delta = currentTail - m_lastTailPosotion;

  // verlet積分で次の位置を計算
  auto nextTail = currentTail
                  // 前フレームの移動を継続する
                  + delta * (1.0f - DragForce)
                  // 親の回転による子ボーンの移動目標
                  + dmath::rotate(m_initLocalTailDir * Stiffiness * 0.01f,
                                  Head->parentWorldRotation())
    // 外力による移動量
    // + external;
    ;

  assert(!std::isnan(nextTail.x));

  auto position = Head->worldPosition();
  nextTail = position + dmath::normalized(nextTail - position) * m_tailLength;

  // auto head = Head.get();
  assert(!std::isnan(nextTail.x));

  // update
  m_currentTailPosotion = nextTail;
  m_lastTailPosotion = currentTail;
  auto newLocalRotation = WorldPosToLocalRotation(nextTail);
  assert(!std::isnan(newLocalRotation.x));
  Head->rotation = newLocalRotation;
  Head->calcWorld(false);
  for (auto& child : Head->children) {
    // ひとつ下まで
    child->calcWorld(false);
  }
}

DirectX::XMFLOAT4
SpringJoint::WorldPosToLocalRotation(const DirectX::XMFLOAT3& nextTail) const
{
  DirectX::XMFLOAT3 localNextTail;
  auto world = Head->parentWorld();
  assert(!std::isnan(world._41));
  auto localInit = Head->localInit;

  DirectX::XMVECTOR det;
  DirectX::XMStoreFloat3(
    &localNextTail,
    DirectX::XMVector3Transform(
      DirectX::XMLoadFloat3(&nextTail),
      DirectX::XMMatrixInverse(
        &det,
        DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&localInit),
                                  DirectX::XMLoadFloat4x4(&world)))));

  assert(DirectX::XMVectorGetX(det) != 0);

  auto r = dmath::rotate_from_to(m_initLocalTailDir, localNextTail);
  assert(!std::isnan(r.x));
  return r;
}

void
SpringSolver::Add(const std::shared_ptr<gltf::Node>& node,
                  float dragForce,
                  float stiffiness)
{

  DirectX::XMFLOAT3 localTailPosition;
  if (node->children.size()) {
    for (auto& child : node->children) {
      localTailPosition = child->translation;
      m_joints.push_back(
        SpringJoint(node, localTailPosition, dragForce, stiffiness));
      break;
    }
  } else {
    auto delta = node->worldPosition() - node->parentWorldPosition();
    auto childPosition =
      node->worldPosition() + dmath::normalized(delta) * 0.07f;

    DirectX::XMStoreFloat3(
      &localTailPosition,
      DirectX::XMVector3Transform(
        DirectX::XMLoadFloat3(&childPosition),
        DirectX::XMMatrixInverse(nullptr,
                                 DirectX::XMLoadFloat4x4(&node->world))));
  }

  for (auto& child : node->children) {
    Add(child, dragForce, stiffiness);
  }
}

void
SpringSolver::Update(Time time)
{
  bool doUpdate = time != m_lastTime;
  m_lastTime = time;
  if (!doUpdate) {
    return;
  }

  for (auto& joint : m_joints) {
    joint.Update();
  }
}

void
SpringSolver::DrawGizmo()
{
  for (auto& joint : m_joints) {
    joint.DrawGizmo();
  }
}

} // namespace vrm
