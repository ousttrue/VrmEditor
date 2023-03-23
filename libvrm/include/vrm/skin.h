#pragma once
#include <DirectXMath.h>
#include <optional>
#include <stdint.h>
#include <string>
#include <vector>

struct Skin {
  std::string name;
  std::vector<uint32_t> joints;
  std::vector<DirectX::XMFLOAT4X4> bindMatrices;
  // bindMatrix * node worldmatrix
  std::vector<DirectX::XMFLOAT4X4> currentMatrices;
  std::optional<uint32_t> root;
};
