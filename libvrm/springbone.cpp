#include "vrm/springbone.h"
#include "vrm/directxmath_util.h"
#include "vrm/scenetypes.h"
#include <iostream>

namespace vrm {

SpringJoint::SpringJoint(const std::shared_ptr<Node> &head,
                         const DirectX::XMFLOAT3 &localTailPosition,
                         float dragForce, float stiffiness)
    : Head(head), DragForce(dragForce), Stiffiness(stiffiness) {

  m_currentTailPosotion = dmath::transform(localTailPosition, head->world);
  m_lastTailPosotion = m_currentTailPosotion;
  m_tailLength = dmath::length(localTailPosition);
  assert(m_tailLength);
  m_initLocalTailDir = dmath::normalized(localTailPosition);
}

void SpringJoint::Update() {
  auto currentTail = m_currentTailPosotion;
  auto prevTail = m_lastTailPosotion;

  // auto delta = currentTail - prevTail;

  // verlet積分で次の位置を計算
  auto nextTail = currentTail
      // 前フレームの移動を継続する
      // + delta * (1.0f - DragForce)
      //     + ParentRotation * LocalRotation * m_boneAxis *
      //           stiffnessForce // 親の回転による子ボーンの移動目標
      //     + external;          // 外力による移動量
      ;

  auto position = Head->worldPosition();
  nextTail = position + dmath::normalized(nextTail - position) * m_tailLength;

  // update
  m_currentTailPosotion = nextTail;
  m_lastTailPosotion = currentTail;

  Head->setWorldRotation(WorldPosToLocalRotation(nextTail));
}

DirectX::XMFLOAT4
SpringJoint::WorldPosToLocalRotation(const DirectX::XMFLOAT3 &nextTail) const {
  auto parent_r = Head->parentWorldRotation();
  auto world_tail_dir = dmath::rotate(m_initLocalTailDir, parent_r);
  auto from_to =
      dmath::rotate_from_to(world_tail_dir, nextTail - Head->worldPosition());
  return dmath::multiply(from_to, parent_r);
}

void SpringSolver::Add(const std::shared_ptr<Node> &node, float dragForce,
                       float stiffiness) {

  DirectX::XMFLOAT3 localTailPosition;
  if (node->children.size()) {
    for (auto &child : node->children) {
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

  for (auto &child : node->children) {
    Add(child, dragForce, stiffiness);
  }
}

void SpringSolver::Update() {
  for (auto &joint : m_joints) {
    joint.Update();
  }
}

} // namespace vrm
