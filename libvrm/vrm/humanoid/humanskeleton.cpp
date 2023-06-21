#include "humanskeleton.h"

namespace libvrm {

DirectX::XMMATRIX
SkeletonBone::Matrix() const
{
  auto r = DirectX::XMMatrixRotationQuaternion(
    DirectX::XMLoadFloat4((const DirectX::XMFLOAT4*)&WorldRotation));
  auto t = DirectX::XMMatrixTranslation(
    WorldPosition[0], WorldPosition[1], WorldPosition[2]);
  return DirectX::XMMatrixMultiply(r, t);
}

DirectX::XMMATRIX
SkeletonBone::InverseMatrix() const
{
  auto r = DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionInverse(
    DirectX::XMLoadFloat4((const DirectX::XMFLOAT4*)&WorldRotation)));
  auto t = DirectX::XMMatrixTranslation(
    -WorldPosition[0], -WorldPosition[1], -WorldPosition[2]);
  return DirectX::XMMatrixMultiply(t, r);
}

std::tuple<std::array<float, 3>, std::array<float, 4>>
SkeletonBone::ToLocal(const SkeletonBone& parent)
{
  auto m = Matrix() * parent.InverseMatrix();
  DirectX::XMVECTOR s;
  DirectX::XMVECTOR r;
  DirectX::XMVECTOR t;
  if (!DirectX::XMMatrixDecompose(&s, &r, &t, m)) {
    return {};
  }
  std::array<float, 3> pos;
  std::array<float, 4> rot;
  DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&pos, t);
  DirectX::XMStoreFloat4((DirectX::XMFLOAT4*)&rot, r);
  return { pos, rot };
}

} // namespace
