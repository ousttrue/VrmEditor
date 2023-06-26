#pragma once
#include "rendering_env.h"
#include <DirectXMath.h>
#include <grapho/camera/camera.h>
#include <grapho/vars.h>

namespace glr {

struct WorldInfo
{
  const grapho::camera::Camera& m_camera;
  const RenderingEnv& m_env;

  DirectX::XMFLOAT4X4 ProjectionMatrix() const
  {
    return m_camera.ProjectionMatrix;
  }
  DirectX::XMFLOAT4X4 ViewMatrix() const { return m_camera.ViewMatrix; }
  DirectX::XMFLOAT4X4 ViewProjectionMatrix() const
  {
    DirectX::XMFLOAT4X4 m;
    DirectX::XMStoreFloat4x4(
      &m,
      DirectX::XMLoadFloat4x4(&m_camera.ViewMatrix) *
        DirectX::XMLoadFloat4x4(&m_camera.ProjectionMatrix));
    return m;
  }
  DirectX::XMFLOAT4X4 ShadowMatrix() const { return m_env.ShadowMatrix; }
  DirectX::XMFLOAT3 CameraPosition() const
  {
    return m_camera.Transform.Translation;
  }
};

struct LocalInfo
{
  DirectX::XMFLOAT4X4 ModelMatrix;
  DirectX::XMFLOAT4X4 NormalMatrix4;
  DirectX::XMFLOAT3X3 NormalMatrix3;
  DirectX::XMFLOAT3X3 IdentityMatrix3;
  bool HasVertexColor;

  LocalInfo(const DirectX::XMFLOAT4X4& model, bool hasVertexColor = false)
    : ModelMatrix(model)
    , HasVertexColor(hasVertexColor)
  {
    auto m = DirectX::XMLoadFloat4x4(&ModelMatrix);
    DirectX::XMVECTOR det;
    auto ti = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, m));
    DirectX::XMStoreFloat4x4(&NormalMatrix4, ti);
    DirectX::XMStoreFloat3x3(&NormalMatrix3, ti);

    DirectX::XMStoreFloat3x3(&IdentityMatrix3, DirectX::XMMatrixIdentity());
  }
};

}
