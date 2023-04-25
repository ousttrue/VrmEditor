#pragma once
#include "gizmo.h"
#include "node.h"
#include "timeline.h"
#include <DirectXMath.h>

namespace libvrm {
namespace vrm {

class SpringJoint
{
public:
  std::shared_ptr<gltf::Node> Head;
  // 減衰[0~1]
  float DragForce = 0;
  // 剛性。初期姿勢への復元力[0~]
  // 増えれば増えるほど比率が高まる。
  float Stiffiness = 0;
  float Radius = 0;

private:
  DirectX::XMFLOAT3 m_currentTailPosotion;
  DirectX::XMFLOAT3 m_lastTailPosotion;
  float m_tailLength;
  DirectX::XMFLOAT3 m_initLocalTailDir;

public:
  SpringJoint(const std::shared_ptr<gltf::Node>& head,
              const DirectX::XMFLOAT3& localTailPosition,
              float dragForce,
              float stiffiness,
              float radius);
  void Update(Time time, struct SpringCollision* collision);
  DirectX::XMVECTOR ConstraintTailPosition(const DirectX::XMVECTOR& src);

  DirectX::XMVECTOR WorldPosToLocalRotation(
    const DirectX::XMVECTOR& nextTail) const;
  void DrawGizmo(IGizmoDrawer* gizmo);
};

}
}
