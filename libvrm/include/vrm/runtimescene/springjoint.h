#pragma once
#include <DirectXMath.h>
#include <vrm/scenetypes.h>
#include <vrm/springcollider.h>
#include <vrm/springjoint.h>

namespace runtimescene {

struct RuntimeSpringCollision;
struct RuntimeScene;

struct RuntimeSpringJoint
{
  std::shared_ptr<libvrm::vrm::SpringJoint> Joint;

  DirectX::XMFLOAT3 m_currentTailPosotion;
  DirectX::XMFLOAT3 m_lastTailPosotion;
  float m_tailLength;
  DirectX::XMFLOAT3 m_initLocalTailDir;

  RuntimeSpringJoint(const std::shared_ptr<libvrm::vrm::SpringJoint>& joint);

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
