#pragma once
#include <DirectXMath.h>

namespace glr {
struct RenderingEnv
{
  float Viewport[4];
  float ClearColor[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
  float ViewMatrix[16];
  float ProjectionMatrix[16];

  // w == 0 ? directional : point
  DirectX::XMFLOAT4 LightPosition = { 2, 2, 2, 0 };
  DirectX::XMFLOAT4X4 ShadowMatrix;

  void Resize(int width, int height)
  {
    Viewport[2] = static_cast<float>(width);
    Viewport[3] = static_cast<float>(height);
  }
  int Width() const { return static_cast<int>(Viewport[2]); }
  int Height() const { return static_cast<int>(Viewport[3]); }

  float PremulR() const { return ClearColor[0] * ClearColor[3]; }
  float PremulG() const { return ClearColor[1] * ClearColor[3]; }
  float PremulB() const { return ClearColor[2] * ClearColor[3]; }
  float Alpha() const { return ClearColor[3]; }
};
}
