#pragma once
#include <DirectXMath.h>
#include <algorithm>
#include <stdint.h>

namespace boneskin {

struct ushort4
{
  uint16_t X;
  uint16_t Y;
  uint16_t Z;
  uint16_t W;
};

struct byte4
{
  uint8_t X;
  uint8_t Y;
  uint8_t Z;
  uint8_t W;
};

struct BoundingBox
{
  DirectX::XMFLOAT3 Min{
    std::numeric_limits<float>::infinity(),
    std::numeric_limits<float>::infinity(),
    std::numeric_limits<float>::infinity(),
  };
  DirectX::XMFLOAT3 Max{
    -std::numeric_limits<float>::infinity(),
    -std::numeric_limits<float>::infinity(),
    -std::numeric_limits<float>::infinity(),
  };

  void Extend(const DirectX::XMFLOAT3& p)
  {
    Min.x = std::min(Min.x, p.x);
    Min.y = std::min(Min.y, p.y);
    Min.z = std::min(Min.z, p.z);
    Max.x = std::max(Max.x, p.x);
    Max.y = std::max(Max.y, p.y);
    Max.z = std::max(Max.z, p.z);
  }
};

} // namespace
