#include "vrm/springbone.h"
#include "vrm/directxmath_util.h"
#include "vrm/scenetypes.h"
#include <iostream>

namespace vrm {

SpringJoint::SpringJoint(const std::shared_ptr<Node> &head,
                         const std::shared_ptr<Node> &tail, float dragForce,
                         float stiffiness)
    : Head(head), Tail(tail), DragForce(dragForce), Stiffiness(stiffiness) {

  std::cout << head->name << "=>" << tail->name << std::endl;

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

  auto from = dmath::normalize(m_initLocalTailDir);
  auto to = dmath::normalize(localDir);
  auto axis = dmath::cross(from, to);
  auto dot = dmath::dot(from, to);
  if (abs(1.0f - dot) < 1E-3) {
    return {0, 0, 0, 1};
  } else {
    auto angle = acos(dot);
    auto q = dmath::axisAngle(axis, angle);
    return q;
  }
}

void SpringJoint::Update() {
  auto currentTail = Tail->worldPosition();
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

  auto position = Head->worldPosition();
  nextTail = position + dmath::normalize(nextTail - position) * m_tailLength;

  // update
  m_lastTailPosotion = currentTail;

  Head->rotation = WorldPosToLocalRotation(nextTail);
  Head->calcWorld();
}

void SpringSolver::Add(const std::shared_ptr<Node> &node, float dragForce,
                       float stiffiness, const std::shared_ptr<Node> &parent) {

  if (parent) {
    m_joints.push_back(SpringJoint(parent, node, dragForce, stiffiness));
  }
  for (auto &child : node->children) {
    Add(child, dragForce, stiffiness, node);
    break;
  }
}

void SpringSolver::Update() {
  for (auto &joint : m_joints) {
    joint.Update();
  }
}

} // namespace vrm
