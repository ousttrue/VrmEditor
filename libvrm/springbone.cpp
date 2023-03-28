#include "vrm/springbone.h"
#include "vrm/directxmath_util.h"
#include "vrm/scenetypes.h"

namespace vrm {

SpringJoint::SpringJoint(const std::shared_ptr<Node> &head,
                         const std::shared_ptr<Node> &tail, float dragForce,
                         float stiffiness)
    : Head(head), Tail(tail), DragForce(dragForce), Stiffiness(stiffiness) {

  Node *h = head.get();
  Node *t = tail.get();

  m_currentTailPosition = tail->worldPosition();
  m_lastTailPosotion = tail->worldPosition();
  m_tailLength = dmath::distance(head->worldPosition(), tail->worldPosition());
  assert(m_tailLength);
  m_initLocalTailDir =
      dmath::normalize(*((DirectX::XMFLOAT3 *)&tail->translation));
}

DirectX::XMFLOAT4
SpringJoint::WorldPosToLocalRotation(const DirectX::XMFLOAT3 &nextTail) const {
  auto worldDir = nextTail - Head->worldPosition();
  auto parent = Head->parentWorldRotation();
  auto inv = DirectX::XMQuaternionInverse(DirectX::XMLoadFloat4(&parent));
  DirectX::XMFLOAT3 localDir;
  DirectX::XMStoreFloat3(&localDir, DirectX::XMVector3Rotate(
                                        DirectX::XMLoadFloat3(&worldDir), inv));
  return dmath::rotate_from_to(m_initLocalTailDir, localDir);
}

void SpringJoint::Update() {
  auto currentTail = m_currentTailPosition;
  auto prevTail = m_lastTailPosotion;

  auto delta = currentTail - prevTail;

  // verlet積分で次の位置を計算
  auto nextTail = currentTail +
                  // 前フレームの移動を継続する
                  delta * (1.0f - DragForce)
      //     + ParentRotation * LocalRotation * m_boneAxis *
      //           stiffnessForce // 親の回転による子ボーンの移動目標
      //     + external;          // 外力による移動量
      ;

  // nextTail = ForceTailPosition(nextTail);
  auto position = Head->worldPosition();
  nextTail = position + dmath::normalize(nextTail - position) * m_tailLength;

  // update
  m_lastTailPosotion = currentTail;
  m_currentTailPosition = nextTail;

  Head->rotation = WorldPosToLocalRotation(nextTail);
  Head->calcWorld();
}

void SpringSolver::Add(const std::shared_ptr<Node> &head,
                       const std::shared_ptr<Node> &tail, float dragForce,
                       float stiffiness) {
  m_joints.push_back(SpringJoint(head, tail, dragForce, stiffiness));
}

void SpringSolver::Update() {
  for (auto &joint : m_joints) {
    joint.Update();
  }
}

} // namespace vrm
