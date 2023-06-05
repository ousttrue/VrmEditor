#pragma once
#include "spring_collider.h"
#include "spring_joint.h"
#include "timeline.h"
#include <DirectXMath.h>

namespace runtimescene {

struct RuntimeSpringCollision;
struct RuntimeScene;

struct RuntimeSpringJoint
{
  std::shared_ptr<libvrm::SpringJoint> Joint;

  DirectX::XMFLOAT3 m_currentTailPosotion;
  DirectX::XMFLOAT3 m_lastTailPosotion;
  float m_tailLength;
  DirectX::XMFLOAT3 m_initLocalTailDir;

  RuntimeSpringJoint(const std::shared_ptr<libvrm::SpringJoint>& joint);

  void Update(RuntimeScene* runtime,
              libvrm::Time time,
              RuntimeSpringCollision* collision);
  DirectX::XMVECTOR ConstraintTailPosition(RuntimeScene* runtime,
                                           const DirectX::XMVECTOR& src);
  DirectX::XMVECTOR WorldPosToLocalRotation(
    RuntimeScene* runtime,
    const DirectX::XMVECTOR& nextTail) const;
  void DrawGizmo(RuntimeScene* runtime, libvrm::IGizmoDrawer* gizmo);
};

}
