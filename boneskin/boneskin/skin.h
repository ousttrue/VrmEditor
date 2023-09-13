#pragma once
#include <DirectXMath.h>
#include <optional>
#include <stdint.h>
#include <string>
#include <vector>

namespace boneskin {

struct Skin
{
  std::string Name;
  std::vector<uint32_t> Joints;
  std::vector<DirectX::XMFLOAT4X4> BindMatrices;
  // bindMatrix * node worldmatrix
  std::vector<DirectX::XMFLOAT4X4> CurrentMatrices;
  std::optional<uint32_t> Root;
};

} // namespace
