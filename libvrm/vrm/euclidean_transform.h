#include <DirectXMath.h>

namespace libvrm {

struct EuclideanTransform
{
  DirectX::XMFLOAT4 Rotation = { 0, 0, 0, 1 };
  DirectX::XMFLOAT3 Translation = { 0, 0, 0 };

  bool HasRotation() const
  {
    if (Rotation.x == 0 && Rotation.y == 0 && Rotation.z == 0 &&
        (Rotation.w == 1 || Rotation.w == -1)) {
      return false;
    }
    return true;
  }

  DirectX::XMMATRIX ScalingTranslationMatrix(float scaling) const
  {
    auto r =
      DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&Rotation));
    auto t = DirectX::XMMatrixTranslation(Translation.x * scaling,
                                          Translation.y * scaling,
                                          Translation.z * scaling);
    return r * t;
  }

  DirectX::XMMATRIX Matrix() const
  {
    auto r =
      DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&Rotation));
    auto t =
      DirectX::XMMatrixTranslation(Translation.x, Translation.y, Translation.z);
    return DirectX::XMMatrixMultiply(r, t);
  }
};

} // namespace
