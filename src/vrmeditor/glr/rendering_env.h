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
  DirectX::XMFLOAT4 Viewport;
  DirectX::XMFLOAT4 ClearColor = { 0.45f, 0.55f, 0.60f, 1.00f };
  DirectX::XMFLOAT4X4 ViewMatrix;
  DirectX::XMFLOAT4X4 ProjectionMatrix;
  DirectX::XMFLOAT3 CameraPosition;

  std::shared_ptr<grapho::gl3::PbrEnv> m_pbr;
  bool LoadPbr(const std::filesystem::path& hdr);
  void RenderSkybox();

  // w == 0 ? directional : point
  DirectX::XMFLOAT4 LightPosition = { 2, 2, 2, 0 };
  DirectX::XMFLOAT4X4 ShadowMatrix;

  void Resize(int width, int height)
  {
    Viewport.z = static_cast<float>(width);
    Viewport.w = static_cast<float>(height);
  }
  int Width() const { return static_cast<int>(Viewport.z); }
  int Height() const { return static_cast<int>(Viewport.w); }

  float PremulR() const { return ClearColor.x * ClearColor.w; }
  float PremulG() const { return ClearColor.y * ClearColor.w; }
  float PremulB() const { return ClearColor.z * ClearColor.w; }
  float Alpha() const { return ClearColor.w; }
};
}
