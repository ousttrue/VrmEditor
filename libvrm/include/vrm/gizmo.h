#pragma once
#include <DirectXMath.h>

namespace libvrm {

struct IGizmoDrawer
{
  virtual ~IGizmoDrawer(){};
  virtual void Fix() = 0;
  virtual void Clear() = 0;
  virtual void DrawLine(const DirectX::XMFLOAT3& p0,
                        const DirectX::XMFLOAT3& p1,
                        const DirectX::XMFLOAT4& color) = 0;
  virtual void DrawSphere(const DirectX::XMFLOAT3& pos,
                          float radius,
                          const DirectX::XMFLOAT4& color) = 0;
  virtual void DrawCapsule(const DirectX::XMFLOAT3& p0,
                           const DirectX::XMFLOAT3& p1,
                           float radius,
                           const DirectX::XMFLOAT4& color) = 0;
};

}
