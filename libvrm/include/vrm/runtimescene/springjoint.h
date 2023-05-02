#pragma once
#include <DirectXMath.h>
#include <vrm/scenetypes.h>
#include <vrm/springcollider.h>
#include <vrm/springjoint.h>

namespace runtimescene {

struct RuntimeSpringCollision;

struct RuntimeSpringJoint
{
  DirectX::XMFLOAT3 m_currentTailPosotion;
  DirectX::XMFLOAT3 m_lastTailPosotion;
  float m_tailLength;
  DirectX::XMFLOAT3 m_initLocalTailDir;

  RuntimeSpringJoint(const std::shared_ptr<libvrm::vrm::SpringJoint>& joint);
  void Update(const libvrm::vrm::SpringJoint& joint,
              libvrm::Time time,
              RuntimeSpringCollision* collision);
  DirectX::XMVECTOR ConstraintTailPosition(
    const libvrm::vrm::SpringJoint& joint,
    const DirectX::XMVECTOR& src);

  DirectX::XMVECTOR WorldPosToLocalRotation(
    const libvrm::vrm::SpringJoint& joint,
    const DirectX::XMVECTOR& nextTail) const;
  void DrawGizmo(const libvrm::vrm::SpringJoint& joint,
                 libvrm::IGizmoDrawer* gizmo);
};

}
