#pragma once
#include <DirectXMath.h>
#include <unordered_map>

namespace boneskin {

struct NodeState
{
  DirectX::XMFLOAT4X4 Matrix;
  std::unordered_map<uint32_t, float> MorphMap;
};

} // namespace
