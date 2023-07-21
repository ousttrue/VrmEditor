#pragma once
#include "spring_collider.h"
#include "spring_joint.h"
#include "timeline.h"
#include <DirectXMath.h>

namespace libvrm {

struct RuntimeSpringCollision;
struct IGizmoDrawer;

struct RuntimeSpringJoint
{
  std::shared_ptr<SpringJoint> Joint;

  DirectX::XMFLOAT3 m_currentTailPosotion;
  DirectX::XMFLOAT3 m_lastTailPosotion;
  float m_tailLength;
  DirectX::XMFLOAT3 m_initLocalTailDir;

  RuntimeSpringJoint(const std::shared_ptr<SpringJoint>& joint);

  void Update(Time time, RuntimeSpringCollision* collision);
  DirectX::XMVECTOR ConstraintTailPosition(const DirectX::XMVECTOR& src);
  DirectX::XMVECTOR WorldPosToLocalRotation(
    const DirectX::XMVECTOR& nextTail) const;
  void DrawGizmo(IGizmoDrawer* gizmo, const DirectX::XMFLOAT4& color);
};

} // namespace
