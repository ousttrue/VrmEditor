#pragma once
#include "dmath.h"
#include <grapho/mesh.h>

namespace libvrm {

struct IGizmoDrawer
{
  virtual ~IGizmoDrawer(){};
  virtual void Fix() = 0;
  virtual void Clear() = 0;
  virtual void DrawLine(const DirectX::XMFLOAT3& p0,
                        const DirectX::XMFLOAT3& p1,
                        const grapho::RGBA& color) = 0;
  virtual void DrawSphere(const DirectX::XMFLOAT3& pos,
                          float radius,
                          const grapho::RGBA& color) = 0;
  virtual void DrawCapsule(const DirectX::XMFLOAT3& p0,
                           const DirectX::XMFLOAT3& p1,
                           float radius,
                           const grapho::RGBA& color) = 0;
};

}
