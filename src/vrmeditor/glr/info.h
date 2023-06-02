#pragma once
#include "rendering_env.h"
#include <DirectXMath.h>
#include <grapho/vars.h>

namespace glr {

struct WorldInfo
{
  const RenderingEnv& m_env;

  DirectX::XMFLOAT4X4 ProjectionMatrix() const
  {
    return m_env.ProjectionMatrix;
  }
  DirectX::XMFLOAT4X4 ViewMatrix() const { return m_env.ViewMatrix; }
  DirectX::XMFLOAT4X4 ViewProjectionMatrix() const
  {
    DirectX::XMFLOAT4X4 m;
    DirectX::XMStoreFloat4x4(
      &m,
      DirectX::XMLoadFloat4x4(&m_env.ViewMatrix) *
        DirectX::XMLoadFloat4x4(&m_env.ProjectionMatrix));
    return m;
  }
  DirectX::XMFLOAT4X4 ShadowMatrix() const { return m_env.ShadowMatrix; }
  DirectX::XMFLOAT3 CameraPosition() const { return m_env.CameraPosition; }
};

struct LocalInfo
{
  const grapho::LocalVars& m_local;

  DirectX::XMFLOAT4X4 ModelMatrix() const { return m_local.model; }
  DirectX::XMFLOAT4X4 NormalMatrix4() const { return m_local.normalMatrix; }
  DirectX::XMFLOAT3X3 NormalMatrix3() const { return m_local.normalMatrix3(); }
  DirectX::XMFLOAT4 ColorRGBA() const { return m_local.color; }
  DirectX::XMFLOAT3 EmissiveRGB() const { return m_local.emissiveColor; }
  DirectX::XMFLOAT3X3 UvTransformMatrix() const
  {
    return m_local.uvTransform();
  }
};

}
