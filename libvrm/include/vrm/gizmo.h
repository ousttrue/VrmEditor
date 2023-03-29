#pragma once
#include <DirectXMath.h>
#include <grapho/mesh.h>
#include <vector>

namespace gizmo {

std::vector<grapho::LineVertex> &lines();
void fix();
void clear();
void drawLine(const DirectX::XMFLOAT3 &p0, const DirectX::XMFLOAT3 &p1,
              const grapho::RGBA &color);
void drawSphere(const DirectX::XMFLOAT3 &pos, const DirectX::XMFLOAT4 &color);

}; // namespace gizmo
