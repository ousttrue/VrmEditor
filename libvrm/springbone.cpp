#include "vrm/springbone.h"
#include "vrm/directxmath_util.h"

namespace vrm {

SpringJoint::SpringJoint(const std::shared_ptr<Node> &head,
                         const std::shared_ptr<Node> &tail, float dragForce,
                         float stiffiness)
    : Head(head), Tail(tail), DragForce(dragForce), Stiffiness(stiffiness) {
  m_lastTailPosotion = tail->worldPosition();
  m_tailLength = dmath::distance(head->worldPosition(), tail->worldPosition());
  m_initLocalTailDir =
      dmath::normalize(*((DirectX::XMFLOAT3 *)&tail->translation));
}

DirectX::XMFLOAT3
SpringJoint::ForceTailPosition(const DirectX::XMFLOAT3 &nextTail) const {
  // 長さをboneLengthに強制することで回転運動化する
  auto position = Head->worldPosition();
  auto _dir = DirectX::XMVector3Normalize(
      dmath::load(dmath::subtract(position, nextTail)));
  auto new_pos =
      dmath::add(position, dmath::multiply(dmath::store(_dir), m_tailLength));
  return new_pos;
}

DirectX::XMFLOAT4
SpringJoint::PosToRotation(const DirectX::XMFLOAT3 &nextTail) const {

  auto parent = Head->parentWorldRotation();
  auto initDir =
      DirectX::XMVector3Rotate(DirectX::XMLoadFloat3(&m_initLocalTailDir),
                               DirectX::XMLoadFloat4(&parent));
  DirectX::XMFLOAT3 tmp;
  DirectX::XMStoreFloat3(&tmp, initDir);
  return dmath::rotate_from_to(tmp, nextTail);
}

void SpringJoint::Update() {
  auto currentTail = Tail->worldPosition();
  auto prevTail = m_lastTailPosotion;

  auto delta = dmath::subtract(currentTail, prevTail);

  // verlet積分で次の位置を計算
  auto nextTail = dmath::add(
      currentTail,
      dmath::multiply(
          delta,
          1.0f - DragForce)) // 前フレームの移動を継続する(減衰もあるよ)
      //     + ParentRotation * LocalRotation * m_boneAxis *
      //           stiffnessForce // 親の回転による子ボーンの移動目標
      //     + external;          // 外力による移動量
      ;

  // update
  m_lastTailPosotion = currentTail;
  auto r = PosToRotation(nextTail);
  Head->rotation = r;
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
