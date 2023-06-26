#pragma once
#include <DirectXMath.h>
#include <filesystem>
#include <memory>
namespace grapho {
namespace gl3 {
struct PbrEnv;
}
}

namespace glr {

struct RenderingEnv
{
  // DirectX::XMFLOAT4 Viewport;
  // DirectX::XMFLOAT4X4 ViewMatrix;
  // DirectX::XMFLOAT4X4 ProjectionMatrix;
  // DirectX::XMFLOAT3 CameraPosition;

  // w == 0 ? directional : point
  DirectX::XMFLOAT4 LightColor = { 1, 1, 1, 1 };
  DirectX::XMFLOAT4 LightPosition = { 2, 2, 2, 0 };
  DirectX::XMFLOAT4X4 ShadowMatrix;

  void SetShadowHeight(float y)
  {
    DirectX::XMFLOAT4 plane = { 0, 1, 0, -y };
    DirectX::XMStoreFloat4x4(
      &ShadowMatrix,
      DirectX::XMMatrixShadow(DirectX::XMLoadFloat4(&plane),
                              DirectX::XMLoadFloat4(&LightPosition)));
  }

  // void Resize(int width, int height)
  // {
  //   Viewport.z = static_cast<float>(width);
  //   Viewport.w = static_cast<float>(height);
  // }
  // int Width() const { return static_cast<int>(Viewport.z); }
  // int Height() const { return static_cast<int>(Viewport.w); }

  DirectX::XMFLOAT4 ClearColor = { 0.45f, 0.55f, 0.60f, 1.00f };
  float PremulR() const { return ClearColor.x * ClearColor.w; }
  float PremulG() const { return ClearColor.y * ClearColor.w; }
  float PremulB() const { return ClearColor.z * ClearColor.w; }
  float Alpha() const { return ClearColor.w; }
};
}
